//
//  BLEConnection.swift
//  DouLamp
//
//  Created by Chen Xu on 2020/10/10.
//

import CoreBluetooth
import Foundation
import UIKit

protocol LampStateSink {
    func lampStateDidChange(connected: Bool, white: UInt8, yellow: UInt8, turnedOn: Bool)
}

class BLEConnection: NSObject, CBPeripheralDelegate, CBCentralManagerDelegate, ObservableObject {
    public static let bleServiceUUID = CBUUID(string: "B6935877-54BA-4F86-ABD7-09A4218799DF")
    public static let bleCharacteristicUUID = CBUUID(string: "3CDB6EAF-EC80-4CCF-9B3E-B78EFA3B28AD")

    // Properties
    private var centralManager: CBCentralManager!
    private var peripheral: CBPeripheral!
    private var characteristic: CBCharacteristic!
    private var connected: Bool = false

    private var white: UInt8 = 0
    private var yellow: UInt8 = 0
    private var turnedOn: UInt8 = 0

    var lampStateSink: LampStateSink?

    // Array to contain names of BLE devices to connect to.
    // Accessable by ContentView for Rendering the SwiftUI Body on change in this array.
    @Published var scannedBLEDevices: [String] = []

    func startCentralManager() {
        centralManager = CBCentralManager(delegate: self, queue: nil)
        print("Central Manager State: \(centralManager.state)")
        DispatchQueue.main.asyncAfter(deadline: .now() + 1) {
            self.centralManagerDidUpdateState(self.centralManager)
        }
    }

    // Handles BT Turning On/Off
    public func centralManagerDidUpdateState(_ central: CBCentralManager) {
        switch central.state {
        case .unsupported:
            print("BLE is Unsupported")
        case .unauthorized:
            print("BLE is Unauthorized")
        case .unknown:
            print("BLE is Unknown")
        case .resetting:
            print("BLE is Resetting")
        case .poweredOff:
            print("BLE is Powered Off")
        case .poweredOn:
            if !centralManager.isScanning {
                print("Central scanning for", BLEConnection.bleServiceUUID)
                centralManager.scanForPeripherals(withServices: [BLEConnection.bleServiceUUID], options: [CBCentralManagerScanOptionAllowDuplicatesKey: true])
            }
            break
        @unknown default:
            print("Unknown state")
        }

        if central.state != CBManagerState.poweredOn {
            print("Bluetooth is not turned on.")
            return
        }
    }

    // Handles the result of the scan
    public func centralManager(_: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData _: [String: Any], rssi RSSI: NSNumber) {
        print("Peripheral Name: \(String(describing: peripheral.name))  RSSI: \(String(RSSI.doubleValue))")
        // We've found it so stop scan
        centralManager.stopScan()
        // Copy the peripheral instance
        self.peripheral = peripheral
        if peripheral.name == nil { return }
        if peripheral.name!.isEmpty { return }
        if scannedBLEDevices.contains(peripheral.name ?? "") { return }
        scannedBLEDevices.append(peripheral.name!)
        self.peripheral.delegate = self
        // Connect!
        centralManager.connect(self.peripheral, options: nil)
    }

    // The handler if we do connect successfully
    public func centralManager(_: CBCentralManager, didConnect peripheral: CBPeripheral) {
        if peripheral == self.peripheral {
            print("Connected to the lamp.")
            peripheral.discoverServices([BLEConnection.bleServiceUUID])
        }
    }

    public func centralManager(_: CBCentralManager, didDisconnectPeripheral _: CBPeripheral, error: Error?) {
        print("Disconnected from the lamp.")
        connected = false
        lampStateSink?.lampStateDidChange(connected: connected, white: white, yellow: yellow, turnedOn: turnedOn > 0)
        if error != nil {
            print("Reason: \(error!)")
        }
        print("Reconnecting...")
        centralManager.connect(peripheral, options: nil)
    }

    // Handles discovery event
    public func peripheral(_ peripheral: CBPeripheral, didDiscoverServices _: Error?) {
        if let services = peripheral.services {
            for service in services {
                if service.uuid == BLEConnection.bleServiceUUID {
                    print("Lamp service found")
                    // Now kick off discovery of characteristics
                    peripheral.discoverCharacteristics([BLEConnection.bleCharacteristicUUID], for: service)
                    return
                }
            }
        }
    }

    // Handling discovery of characteristics
    public func peripheral(_: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error _: Error?) {
        if let characteristics = service.characteristics {
            var found = false
            for characteristic in characteristics {
                print("\(characteristic) vs. \(BLEConnection.bleCharacteristicUUID)")
                if characteristic.uuid == BLEConnection.bleCharacteristicUUID {
                    found = true
                    self.characteristic = characteristic
                    break
                }
            }
            if found {
                print("Lamp characteristic found.")
                peripheral.delegate = self
                connected = true
                peripheral.setNotifyValue(true, for: characteristic)
                peripheral.readValue(for: characteristic)
            } else {
                print("Lamp characteristic not found.")
            }
        }
    }

    public func peripheral(_: CBPeripheral, didWriteValueFor _: CBDescriptor, error: Error?) {
        if error == nil {
            print("Lamp state sent.")
            peripheral.readValue(for: characteristic)
        }
    }

    public func peripheral(_: CBPeripheral, didUpdateValueFor: CBCharacteristic, error _: Error?) {
        let bytes = [UInt8](didUpdateValueFor.value!)
        white = bytes[0]
        yellow = bytes[1]
        turnedOn = bytes[2]
        print("Lamp state refreshed: white=\(white), yellow=\(yellow), turnedOn=\(turnedOn).")
        lampStateSink?.lampStateDidChange(connected: connected, white: white, yellow: yellow, turnedOn: turnedOn > 0)
    }

    public func updateLamp(white: UInt8, yellow: UInt8, turnedOn: UInt8) {
        self.white = white
        self.yellow = yellow
        self.turnedOn = turnedOn

        if connected {
            print("Sending Lamp state: white=\(white), yellow=\(yellow), turnedOn=\(turnedOn).")
            let data = Data(bytes: [white, yellow, turnedOn], count: 3)
            peripheral.writeValue(data, for: characteristic, type: .withResponse)
        } else {
            print("Updating failed, lamp is not connected.")
        }
    }
}
