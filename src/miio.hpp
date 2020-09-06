#pragma once

// TODO: finish up miio implementation

#include <AES.h>
#include <CBC.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <md5.h>

#include "state.hpp"

namespace miio {
WiFiUDP Udp;
unsigned int miioPort = 54321;
CBC<AES128> cbc;

struct packet {
    char magic[2];
    uint16_t length;
    uint32_t unknown;
    char device_id[4];
    uint32_t timestamp;
    uint8_t checksum[16];
    char data[127];
};

uint8_t packet_header_size = 32;

packet incomingPacket;
packet outgoingPacket;

void setup_aes() {
    MD5Builder md5;
    uint8_t key[16];
    md5.begin();
    md5.addHexString(state::TOKEN);
    md5.calculate();
    md5.getBytes(key);

    uint8_t iv[16];
    md5.begin();
    md5.add(key, 16);
    md5.addHexString(state::TOKEN);
    md5.calculate();
    md5.getBytes(iv);

    cbc.setKey(key, 16);
    cbc.setIV(iv, 16);
}

void setup() {
    setup_aes();
    Udp.begin(miioPort);
}

void handle() {
    int packetSize = Udp.parsePacket();
    if (packetSize) {
        // receive incoming UDP packets
        // logger::debugf(
        //     "Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
        // int len = Udp.read((byte *) &incomingPacket, sizeof packet);
        // if (len >= packet_header_size) {
        //     incomingPacket.data[len - packet_header_size] = 0;
        //     logger::debug(packet.data);
        // }
        // logger::debugf("UDP packet contents: %s\n", incomingPacket);

        // send back a reply, to the IP address and port we got the packet from
        // Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        // Udp.write("replyPacket"); // TODO: implementation
        // Udp.endPacket();
    }
}

void decrypt() {
    // cbc.decrypt(output, input, len);
}
} // namespace miio
