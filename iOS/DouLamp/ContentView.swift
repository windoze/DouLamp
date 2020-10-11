//
//  ContentView.swift
//  DouLamp
//
//  Created by Chen Xu on 2020/10/10.
//

import SwiftUI

struct ContentView: View, LampStateSink {
    var body: some View {
        VStack(spacing: 20) {
            Image("lamp180")
            Divider()
            HStack {
                Label("白光", systemImage: "bolt.fill")
                    .labelStyle(TitleOnlyLabelStyle())
                Slider(value: $sliderWhite, in: 0 ... 1, onEditingChanged: sliderChanged)
            }.padding()
            HStack {
                Label("暖光", systemImage: "bolt.fill")
                    .labelStyle(TitleOnlyLabelStyle())
                Slider(value: $sliderYellow, in: 0 ... 1, onEditingChanged: sliderChanged)
            }.padding()
            Divider()
            Button(action: buttonClicked) {
                Text(self.buttonText)
                    .font(.system(size: 80))
            }
            Spacer()
            Text(verbatim: deviceText)
                .onAppear(perform: {
                    if !self.firstAppear { return }
                    bleConnection.lampStateSink = self
                    bleConnection.startCentralManager()
                    self.firstAppear = false
                })
            Text(self.stateText).font(.system(size: 18)).padding()
        }.padding(.top).disabled(!self.connected)
    }

    @State private var connected: Bool = false
    @State private var sliderWhite: Double = 0.5
    @State private var sliderYellow: Double = 0.5
    @State private var deviceText: String = "正在连接台灯……"
    @State private var stateText: String = ""
    @State private var buttonText: String = ""
    @State private var turnedOn: Bool = false
    @State private var firstAppear: Bool = true

    @ObservedObject var bleConnection = BLEConnection()

    private func sliderChanged(_: Bool) {
        print("Slider white \(sliderWhite),  slider yello \(sliderYellow)")
        if turnedOn {
            updateLamp()
        }
    }

    private func buttonClicked() {
        turnedOn = !turnedOn
        updateLamp()
    }

    private func updateLamp() {
        if !connected {
            print("Lamp is not connected.")
            return
        }

        let whiteValue = UInt8(255 * sliderWhite)
        let yellowValue = UInt8(255 * sliderYellow)
        let turnedOnValue: UInt8 = turnedOn ? 1 : 0
        debugPrint("Sending white \(whiteValue), yellow \(yellowValue) to the device.")
        bleConnection.updateLamp(white: whiteValue, yellow: yellowValue, turnedOn: turnedOnValue)
        if turnedOn {
            buttonText = "关灯"
            stateText = "台灯已打开"
        } else {
            buttonText = "开灯"
            stateText = "台灯已关闭"
        }
    }

    func lampStateDidChange(connected: Bool, white: UInt8, yellow: UInt8, turnedOn: Bool) {
        self.connected = connected
        if !connected {
            buttonText = ""
            stateText = ""
            deviceText = "正在连接台灯……"
            return
        }

        deviceText = "已连接到 " + bleConnection.scannedBLEDevices.joined(separator: ",")
        sliderWhite = Double(white) / 255
        sliderYellow = Double(yellow) / 255
        self.turnedOn = turnedOn
        if turnedOn {
            buttonText = "关灯"
            stateText = "台灯已打开"
        } else {
            buttonText = "开灯"
            stateText = "台灯已关闭"
        }
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
