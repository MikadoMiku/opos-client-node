#include <napi.h>

// If you start trying to develop this project you need to look over additional include directories
// Under properties->C/C++->General->Additional includes

// You need CCO_s from monroecs - OPOS_CCOs_1.14.001
// Also need to yarn node_addon_api and include the path to the extra include directories where napi.h is located

// To use CCO_s you need ocx or dll files. First use the #import directive to generate a tlh file
// Then comment out the #import and #include the generated tlh file from the projects x64 directory

// Napi.h requires node_api.h which is usually located where your node version are kept
// Example path of node_api.h: C:\Users\power\AppData\Local\node-gyp\Cache\16.14.2\include\node
// Add the path to the projects extra include directories

// Also need node_addon_api from node_modules

// Node api also needs the library called node.lib which needs to be linked under properties->Linker->Input->Additional Dependencies
// The path for this lib for me was: C:\Users\power\AppData\Local\node-gyp\Cache\16.14.2\x64\node.lib (make sure to add the file with its extension not the directory)

// Example repo of a Scale and Scanner: https://github.com/datalogic/OPOSSamples
// Another example: https://github.com/microsoft/Windows-universal-samples/blob/main/Samples/BarcodeScanner/cpp/Scenario1_BasicFunctionality.xaml.cpp

#include "Oposdevicemanageratl_i.h" // Include the header file generated by your ATL project
#include "Oposdevicemanageratl_i.c"
#include <iostream>
#include <Windows.h>
#include <atlbase.h>
#include <atlcom.h>
#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <ctime>

// Map that ties commandID to a promise that needs resolving
#include <unordered_map>
// std::unordered_map<std::string, Napi::Promise::Deferred> commandPromises;
std::unordered_map<std::string, std::shared_ptr<Napi::Promise::Deferred>> commandPromises;
std::mutex commandPromisesMutex;
// ------

std::queue<std::pair<std::string, std::string>> commandQueue;
std::mutex commandQueueMutex;

std::queue<std::string> completedCommands;
std::mutex completedCommandsMutex;

std::string GenerateUniqueCommandId(std::string command)
{
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%d%m%Y%H%M%S", timeinfo);
    std::string str(buffer);
    return command + "_" + str;
}

class CommandCompletionPoller : public Napi::AsyncWorker
{
public:
    CommandCompletionPoller(Napi::Env &env)
        : Napi::AsyncWorker(env), env_(env) {}

    void Execute()
    {
        std::cout << "Started polling worker" << std::endl;
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // polling interval

            std::lock_guard<std::mutex> lock(completedCommandsMutex);
            if (!completedCommands.empty())
            {
                completedCommandId = completedCommands.front();
                completedCommands.pop();
                break;
            }
        }
    }

    void OnOK()
    {
        Napi::HandleScope scope(env_);

        std::lock_guard<std::mutex> lock(commandPromisesMutex);
        auto it = commandPromises.find(completedCommandId);
        if (it != commandPromises.end())
        {
            it->second->Resolve(Napi::String::New(env_, "Command completed: " + it->first));
            commandPromises.erase(it);
        }
    }

private:
    std::string completedCommandId;
    Napi::Env env_;
};

class CMyDeviceManagerEvents : public IMyDeviceManagerEvents
{
public:
    STDMETHODIMP OnDataEvent(BSTR data)
    {
        std::wcout << std::endl;
        std::wstring wstrData(data);
        std::wcout << L"Data event received: " << wstrData << std::endl;
        return S_OK;
    }

    STDMETHODIMP OnCommandCompleted(BSTR commandId)
    {
        std::wcout << std::endl;
        std::wstring wstrCommandId(commandId);
        std::string strCommandId(wstrCommandId.begin(), wstrCommandId.end());
        std::wcout << L"Command finished: " << wstrCommandId << std::endl;
        {
            std::lock_guard<std::mutex> lock(completedCommandsMutex);
            completedCommands.push(strCommandId);
        }

        return S_OK;
    }

    STDMETHODIMP OnErrorEvent(BSTR errorMessage)
    {
        std::wstring wstrErrorMessage(errorMessage);
        std::wcerr << L"Error event received: " << wstrErrorMessage << std::endl;
        return S_OK;
    }

    STDMETHODIMP_(ULONG)
    AddRef() override { return 1; }
    STDMETHODIMP_(ULONG)
    Release() override { return 1; }

    STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject) override
    {
        if (iid == IID_IUnknown || iid == IID_IMyDeviceManagerEvents)
        {
            *ppvObject = this;
            AddRef();
            return S_OK;
        }
        else
        {
            *ppvObject = NULL;
            return E_NOINTERFACE;
        }
    }
};

void comThreadFunction()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        std::cerr << "Failed to initialize COM library." << std::endl;
        return;
    }

    CComPtr<IOposDeviceManager> spDeviceManager;
    hr = spDeviceManager.CoCreateInstance(__uuidof(OposDeviceManager));
    // ...

    // Create an instance of your event handler class
    CMyDeviceManagerEvents eventHandler;

    // Query the COM object for the IConnectionPointContainer interface
    CComPtr<IConnectionPointContainer> spCPC;
    hr = spDeviceManager->QueryInterface(IID_IConnectionPointContainer, (void **)&spCPC);

    // Find the connection point for the IMyDeviceManagerEvents interface
    CComPtr<IConnectionPoint> spCP;
    hr = spCPC->FindConnectionPoint(IID_IMyDeviceManagerEvents, &spCP);

    // Advise the connection point with the event handler object
    DWORD dwCookie;
    hr = spCP->Advise(&eventHandler, &dwCookie);

    // Call a method of your COM server, e.g., StartScanner
    // BSTR commandId = SysAllocString(L"startScanner");

    // hr = spDeviceManager->StartScanner(commandId);
    // SysFreeString(commandId);

    // Run a loop to handle COM events and user commands
    MSG msg;
    while (true)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        std::pair<std::string, std::string> command;
        {
            std::lock_guard<std::mutex> lock(commandQueueMutex);
            if (!commandQueue.empty())
            {
                command = commandQueue.front();
                commandQueue.pop();
            }
        }

        if (!command.first.empty())
        {

            if (command.first == "stopScanner")
            {
                std::wstring wstrCommandId(command.second.begin(), command.second.end());
                BSTR commandId = SysAllocString(wstrCommandId.c_str());

                hr = spDeviceManager->StopScanner(commandId);
                SysFreeString(commandId);
                break;
            }
            else if (command.first == "enableDataEvents")
            {
                BSTR bstr = SysAllocString(L"device");

                std::wstring wstrCommandId(command.second.begin(), command.second.end());
                BSTR commandId = SysAllocString(wstrCommandId.c_str());

                hr = spDeviceManager->EnableDataEvent(bstr, commandId);
                SysFreeString(bstr);
                SysFreeString(commandId);
            }
            else if (command.first == "disableDataEvents")
            {
                BSTR bstr = SysAllocString(L"device");
                std::wstring wstrCommandId(command.second.begin(), command.second.end());
                BSTR commandId = SysAllocString(wstrCommandId.c_str());
                hr = spDeviceManager->DisableDataEvent(bstr, commandId);
                SysFreeString(bstr);
                SysFreeString(commandId);
            }
            else if (command.first == "startScanner")
            {
                std::wstring wstrCommandId(command.second.begin(), command.second.end());
                BSTR commandId = SysAllocString(wstrCommandId.c_str());

                hr = spDeviceManager->StartScanner(commandId);
                SysFreeString(commandId);
            }

            // Process the command and call the appropriate methods on the COM server
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    hr = spCP->Unadvise(dwCookie);

    CoUninitialize();
}

void inputThreadFunction()
{
    std::string input;
    while (true)
    {
        std::getline(std::cin, input);

        if (input.empty())
        {
            continue;
        }

        std::lock_guard<std::mutex> lock(commandQueueMutex);
        commandQueue.push({input, GenerateUniqueCommandId(input)});

        if (input == "q")
        {
            break;
        }
    }
}

Napi::Promise DoCommand(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    std::string command = info[0].ToString().Utf8Value();
    std::string commandId = GenerateUniqueCommandId(command); // You'll need to implement this function

    std::shared_ptr<Napi::Promise::Deferred> deferred = std::make_shared<Napi::Promise::Deferred>(Napi::Promise::Deferred::New(env));

    {
        std::lock_guard<std::mutex> lock(commandPromisesMutex);
        commandPromises[commandId] = deferred;
    }

    // Enqueue the command along with the command ID
    {
        std::lock_guard<std::mutex> lock(commandQueueMutex);
        commandQueue.push({command, commandId});
    }

    // Start a command completion poller
    CommandCompletionPoller *worker = new CommandCompletionPoller(env);
    worker->Queue();

    return deferred->Promise();

    /*     std::lock_guard<std::mutex> lock(commandQueueMutex);
        commandQueue.push(command); */
}

int mainy()
{
    std::thread comThread(comThreadFunction);
    std::thread inputThread(inputThreadFunction);

    comThread.join();
    inputThread.join();

    std::cout << "Done." << std::endl;
    return 0;
}

Napi::Promise StartMain(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

    try
    {
        std::thread comThread(comThreadFunction);
        std::thread inputThread(inputThreadFunction);

        // comThread.join();
        comThread.detach();
        // inputThread.join();
        inputThread.detach();
    }
    catch (const std::exception &e)
    {
        deferred.Reject(Napi::String::New(env, e.what()));
    }

    return deferred.Promise();
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.Set(Napi::String::New(env, "startMain"),
                Napi::Function::New(env, StartMain));
    exports.Set(Napi::String::New(env, "doCommand"),
                Napi::Function::New(env, DoCommand));
    return exports;
}

NODE_API_MODULE(hello, Init)
