#include "components/smart_desk/rx_decoder.h"
#include "components/smart_desk/tx_verifier.h"

#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
    using esphome::nicode_smart_desk::RxDecoder;
    using esphome::nicode_smart_desk::TxVerifier;

    constexpr uint8_t SEG_0 = 0x3F;
    constexpr uint8_t SEG_1 = 0x06;
    constexpr uint8_t SEG_2 = 0x5B;
    constexpr uint8_t SEG_6 = 0x7D;
    constexpr uint8_t SEG_7 = 0x07;

    void fail(const std::string &message)
    {
        std::cerr << "FAIL: " << message << std::endl;
        std::exit(1);
    }

    void expect(bool condition, const std::string &message)
    {
        if (!condition)
        {
            fail(message);
        }
    }

    void expect_close(float actual, float expected, const std::string &message)
    {
        if (std::fabs(actual - expected) > 0.01f)
        {
            fail(message + " expected " + std::to_string(expected) + " got " + std::to_string(actual));
        }
    }

    std::array<uint8_t, 5> rx_frame(uint8_t screen1, uint8_t screen2, uint8_t screen3)
    {
        return {0x5A, screen1, screen2, screen3,
                static_cast<uint8_t>((screen1 + screen2 + screen3) & 0xFF)};
    }

    std::array<uint8_t, 5> tx_frame(uint8_t pressed)
    {
        const uint8_t released = static_cast<uint8_t>(~pressed);
        return {0xA5, 0x00, pressed, released,
                static_cast<uint8_t>((pressed + released) & 0xFF)};
    }

    bool feed_rx(RxDecoder &decoder, const std::array<uint8_t, 5> &frame)
    {
        bool accepted = false;
        for (uint8_t byte : frame)
        {
            accepted = decoder.put(byte);
        }
        return accepted;
    }

    bool feed_tx(TxVerifier &verifier, const std::array<uint8_t, 5> &frame)
    {
        bool accepted = false;
        for (uint8_t byte : frame)
        {
            accepted = verifier.put(byte);
        }
        return accepted;
    }

    void test_rx_height_and_gap_filter()
    {
        RxDecoder decoder;
        expect(!decoder.put(0x00), "noise byte should not produce an RX frame");

        expect(feed_rx(decoder, rx_frame(SEG_6, SEG_2 | 0x80, SEG_0)), "62.0cm RX frame should be accepted");
        expect_close(decoder.get_desk_height(), 62.0f, "height should decode as 62.0cm");
        expect(decoder.get_desk_display() == "62.0", "display should decode as 62.0");

        expect(feed_rx(decoder, rx_frame(SEG_1, SEG_2, SEG_7)), "127cm RX frame should be accepted");
        expect_close(decoder.get_desk_height(), 62.0f, "large in-range jump should be ignored");

        decoder.set_height_range(70.0f, 120.0f);
        expect(feed_rx(decoder, rx_frame(SEG_7, SEG_1 | 0x80, SEG_0)), "71.0cm RX frame should be accepted");
        expect_close(decoder.get_desk_height(), 71.0f, "range setter should let an out-of-range old height recover");
    }

    void test_rx_checksum_recovery()
    {
        RxDecoder decoder;
        auto bad = rx_frame(SEG_6, SEG_2 | 0x80, SEG_0);
        bad[4] ^= 0x55;
        expect(!feed_rx(decoder, bad), "bad RX checksum should be rejected");

        expect(feed_rx(decoder, rx_frame(SEG_6, SEG_2 | 0x80, SEG_0)), "decoder should recover after bad RX checksum");
        expect_close(decoder.get_desk_height(), 62.0f, "height should update after checksum recovery");
    }

    void test_tx_verifier()
    {
        TxVerifier verifier;
        expect(!verifier.put(0x00), "noise byte should not produce a TX frame");
        expect(feed_tx(verifier, tx_frame(0x20)), "valid TX up frame should be accepted");

        auto bad = tx_frame(0x40);
        bad[4] = 0x00;
        expect(!feed_tx(verifier, bad), "bad TX checksum should be rejected");
        expect(feed_tx(verifier, tx_frame(0x40)), "verifier should recover after bad TX checksum");
    }
}

int main()
{
    test_rx_height_and_gap_filter();
    test_rx_checksum_recovery();
    test_tx_verifier();
    std::cout << "protocol decoder tests passed" << std::endl;
    return 0;
}
