// DOM Elements
const bleStateContainer = document.getElementById('bleState');
const connectButton = document.getElementById('connectBleButton');
const disconnectButton = document.getElementById('disconnectBleButton');
const playButtons = document.querySelectorAll('.play-button');
const slotInputs = document.querySelectorAll('.slot-input');
const timestampContainer = document.getElementById('timestamp');
const scrollSpeedInput = document.getElementById('scrollSpeed');
const colorSwatches = document.querySelectorAll('.color-swatch');
const clearAllButton = document.getElementById('clearAllButton');

// Define BLE Device Specs
var deviceName = 'unsortedLED';
var bleService = '54a145a2-5493-483a-8b10-be70e00e3c93';
var bleCharacteristic = '54a145a3-5493-483a-8b10-be70e00e3c93';

// Global Variables to Handle Bluetooth
var bleDevice;
var bleServer;
var bleServiceFound;
var bleCharacteristicFound;
let speedSendTimer = null;

// Connect Button (search for BLE Devices only if BLE is available)
connectButton.addEventListener('click', () => {
    if (isWebBluetoothEnabled()) {
        connectToDevice();
    }
});

// Disconnect Button
disconnectButton.addEventListener('click', disconnectDevice);

if (clearAllButton) {
    clearAllButton.addEventListener('click', () => {
        slotInputs.forEach((input) => {
            input.value = '';
        });
    });
}

if (scrollSpeedInput) {
    scrollSpeedInput.addEventListener('input', () => {
        if (speedSendTimer) {
            clearTimeout(speedSendTimer);
        }
        speedSendTimer = setTimeout(() => {
            sendSpeedToDevice(scrollSpeedInput.value);
        }, 120);
    });
    scrollSpeedInput.addEventListener('change', () => {
        sendSpeedToDevice(scrollSpeedInput.value);
    });
}

if (colorSwatches.length > 0) {
    colorSwatches.forEach((swatch) => {
        swatch.addEventListener('click', () => {
            setActiveColorSwatch(swatch);
            sendColorToDevice(swatch.dataset.color);
        });
    });
}

// Play Buttons
playButtons.forEach((button) => {
    button.addEventListener('click', () => {
        const specialAction = button.dataset.special;
        if (specialAction === 'love') {
            sendLoveToDevice();
            return;
        }
        const inputA = document.getElementById(button.dataset.inputA);
        const inputB = document.getElementById(button.dataset.inputB);
        const valueA = inputA ? inputA.value : '';
        const valueB = inputB ? inputB.value : '';
        sendSlotToDevice(button.dataset.slot, valueA, valueB);
    });
});

// Check if BLE is available in your Browser
function isWebBluetoothEnabled() {
    if (!navigator.bluetooth) {
        console.log('Web Bluetooth API is not available in this browser!');
        bleStateContainer.innerHTML = 'Web Bluetooth API is not available in this browser!';
        bleStateContainer.style.color = '#d13a30';
        bleStateContainer.style.backgroundColor = 'white';
        return false;
    }
    console.log('Web Bluetooth API supported in this browser.');
    return true;
}

// Connect to BLE Device
async function connectToDevice() {
    console.log('Initializing Bluetooth...');
    bleStateContainer.innerHTML = 'Connecting...';
    bleStateContainer.style.color = '#f5a623';
    bleStateContainer.style.backgroundColor = 'white';

    try {
        const device = await navigator.bluetooth.requestDevice({
            filters: [{ name: deviceName }],
            optionalServices: [bleService]
        });
        console.log('Device Selected:', device.name);
        bleDevice = device;
        bleStateContainer.innerHTML = 'Connecting to GATT server...';
        device.addEventListener('gattserverdisconnected', onDisconnected);
        const gattServer = await device.gatt.connect();

        bleServer = gattServer;
        bleStateContainer.innerHTML = 'Discovering services...';

        console.log('Looking for service:', bleService);
        const service = await bleServer.getPrimaryService(bleService);
        bleServiceFound = service;
        console.log('Service discovered:', service.uuid);

        console.log('Looking for characteristic:', bleCharacteristic);
        const characteristic = await service.getCharacteristic(bleCharacteristic);
        bleCharacteristicFound = characteristic;
        console.log('Characteristic discovered:', characteristic.uuid);

        // Now we are fully connected
        bleStateContainer.innerHTML = 'Connected to ' + bleDevice.name;
        bleStateContainer.style.color = '#24af37';
        bleStateContainer.style.backgroundColor = 'white';

        // Sync current UI settings to the display on new connection.
        if (scrollSpeedInput) {
            sendSpeedToDevice(scrollSpeedInput.value);
        }
        if (colorSwatches.length > 0) {
            const activeSwatch = document.querySelector('.color-swatch.color-active');
            if (activeSwatch) {
                sendColorToDevice(activeSwatch.dataset.color);
            }
        }
    } catch (error) {
        console.error('Connection Error:', error);
        console.error('Error name:', error.name);
        console.error('Error message:', error.message);
        bleStateContainer.innerHTML = 'Failed: ' + error.message;
        bleStateContainer.style.color = '#d13a30';
        bleStateContainer.style.backgroundColor = 'white';
        // Reset state on error
        bleDevice = null;
        bleServer = null;
        bleServiceFound = null;
        bleCharacteristicFound = null;
    }
}

function onDisconnected(event) {
    console.log('Device Disconnected:', event.target.name);
    bleStateContainer.innerHTML = 'Disconnected';
    bleStateContainer.style.color = '#d13a30';
    bleStateContainer.style.backgroundColor = 'white';
    bleDevice = null;
    bleServer = null;
    bleServiceFound = null;
    bleCharacteristicFound = null;
}

async function writeToDevice(message) {
    if (bleDevice && bleDevice.gatt.connected && bleCharacteristicFound) {
        try {
            const encoder = new TextEncoder();
            const data = encoder.encode(message);
            await bleCharacteristicFound.writeValue(data);
            console.log('Message written to device:', message);
            timestampContainer.innerHTML = 'Last sent: ' + message + ' (' + getDateTime() + ')';
        } catch (error) {
            console.error('Error writing to device:', error);
            window.alert('Error writing to device: ' + error.message);
        }
    } else {
        console.error('Bluetooth is not connected. Cannot write to device.');
        window.alert('Bluetooth is not connected. Cannot write to device.\nConnect to BLE first!');
    }
}

function disconnectDevice() {
    console.log('Disconnect Device.');
    if (bleDevice && bleDevice.gatt.connected) {
        bleDevice.gatt.disconnect();
        console.log('Device Disconnected');
        bleStateContainer.innerHTML = 'Disconnected';
        bleStateContainer.style.color = '#d13a30';
        bleStateContainer.style.backgroundColor = 'white';
        bleDevice = null;
        bleServer = null;
        bleServiceFound = null;
        bleCharacteristicFound = null;
    } else {
        console.error('Bluetooth is not connected.');
        window.alert('Bluetooth is not connected.');
    }
}

function getDateTime() {
    var currentdate = new Date();
    var day = ('00' + currentdate.getDate()).slice(-2);
    var month = ('00' + (currentdate.getMonth() + 1)).slice(-2);
    var year = currentdate.getFullYear();
    var hours = ('00' + currentdate.getHours()).slice(-2);
    var minutes = ('00' + currentdate.getMinutes()).slice(-2);
    var seconds = ('00' + currentdate.getSeconds()).slice(-2);

    var datetime = day + '/' + month + '/' + year + ' at ' + hours + ':' + minutes + ':' + seconds;
    return datetime;
}

function sendSlotToDevice(slot, valueA, valueB) {
    const firstText = valueA.trim();
    const secondText = valueB.trim();
    const isBreakSlot = (slot || '').toUpperCase() === 'BREAK';
    const payload = isBreakSlot ? (secondText || firstText) : (firstText + ' x ' + secondText);
    writeToDevice(payload);
}

function sendLoveToDevice() {
    writeToDevice('<3 <3 <3 <3 <3 <3 <3 <3');
}

function sendSpeedToDevice(value) {
    const parsed = Number.parseInt(value, 10);
    if (Number.isNaN(parsed)) {
        return;
    }
    if (!(bleDevice && bleDevice.gatt.connected && bleCharacteristicFound)) {
        return;
    }
    const speed = Math.min(30, Math.max(0, parsed));
    scrollSpeedInput.value = String(speed);
    writeToDevice('CFG:SPEED:' + speed);
}

function sendColorToDevice(hexColor) {
    if (!hexColor) {
        return;
    }
    if (!(bleDevice && bleDevice.gatt.connected && bleCharacteristicFound)) {
        return;
    }
    const normalized = hexColor.trim();
    if (!/^#[0-9a-fA-F]{6}$/.test(normalized)) {
        return;
    }
    writeToDevice('CFG:COLOR:' + normalized.toUpperCase());
}

function setActiveColorSwatch(activeSwatch) {
    colorSwatches.forEach((swatch) => {
        swatch.classList.toggle('color-active', swatch === activeSwatch);
    });
}
