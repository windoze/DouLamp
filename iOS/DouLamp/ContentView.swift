//
//  ContentView.swift
//  DouLamp
//
//  Created by Chen Xu on 2020/10/10.
//

import SwiftUI

struct ContentView: View {
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
            Text(verbatim: "已连接到 " + bleConnection.scannedBLEDevices.joined(separator: ","))
                .onAppear(perform: {
                    if !self.firstAppear { return }
                    bleConnection.view = self
                    bleConnection.startCentralManager()
                    self.firstAppear = false
                })
            Text(self.stateText).font(.system(size: 18)).padding()
        }.padding(.top)
    }

    @State private var sliderWhite: Double = 0.5
    @State private var sliderYellow: Double = 0.5
    @State private var stateText: String = "台灯已关闭"
    @State private var buttonText: String = "开灯"
    @State private var turnedOn: Bool = false
    @State private var firstAppear: Bool = true

    @ObservedObject var bleConnection = BLEConnection()

    func sliderChanged(_: Bool) {
        debugPrint("Slider white \(sliderWhite),  slider yello \(sliderYellow)")
        if turnedOn {
            updateLamp()
        }
    }

    func buttonClicked() {
        turnedOn = !turnedOn
        updateLamp()
    }

    func updateLamp() {
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

    func readLamp(white: UInt8, yellow: UInt8, turnedOn: UInt8) {
        sliderWhite = Double(white) / 255
        sliderYellow = Double(yellow) / 255
        self.turnedOn = turnedOn > 0
        if turnedOn > 0 {
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
