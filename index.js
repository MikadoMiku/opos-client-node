// JavaScript code
const monkiAddon = require("./build/Release/monki.node")

monkiAddon.startMain()

const testFunction = async () => {
    try {
        let result = await monkiAddon.doCommand("startScanner")
        console.log("command 1 completed with result: ", result)

        result = await monkiAddon.doCommand("stopScanner")
        console.log("command 2 completed with result: ", result)
    } catch (error) {
        console.error("Command failed:", error)
    }
}

testFunction()
