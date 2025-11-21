#include <unity.h>

// Clock source definitions from Sync.h
enum ClockSource {
    CLOCK_SOURCE_NONE = 0,
    CLOCK_SOURCE_USB = 1,
    CLOCK_SOURCE_DIN = 2,
    CLOCK_SOURCE_SYNC_IN = 3
};

// Simplified priority logic from Sync.cpp
class ClockPriorityManager {
private:
    ClockSource activeSource;
    bool usbIsPlaying;
    
public:
    ClockPriorityManager() : activeSource(CLOCK_SOURCE_NONE), usbIsPlaying(false) {}
    
    // Simulate receiving clock from a source
    bool acceptClock(ClockSource source) {
        // DIN is rejected if USB or SYNC_IN is active
        if (source == CLOCK_SOURCE_DIN && 
            (activeSource == CLOCK_SOURCE_USB || activeSource == CLOCK_SOURCE_SYNC_IN)) {
            return false;
        }
        
        // USB is rejected if SYNC_IN is active
        if (source == CLOCK_SOURCE_USB && activeSource == CLOCK_SOURCE_SYNC_IN) {
            return false;
        }
        
        // Accept the clock and update active source
        activeSource = source;
        if (source == CLOCK_SOURCE_USB) {
            usbIsPlaying = true;
        }
        return true;
    }
    
    void stop(ClockSource source) {
        if (source == CLOCK_SOURCE_USB) {
            usbIsPlaying = false;
        }
        if (activeSource == source) {
            activeSource = CLOCK_SOURCE_NONE;
        }
    }
    
    ClockSource getActiveSource() const {
        return activeSource;
    }
    
    void reset() {
        activeSource = CLOCK_SOURCE_NONE;
        usbIsPlaying = false;
    }
};

ClockPriorityManager manager;

// Test priority: SYNC_IN blocks USB
void test_priority_sync_in_blocks_usb() {
    manager.reset();
    
    // Start with USB
    TEST_ASSERT_TRUE(manager.acceptClock(CLOCK_SOURCE_USB));
    TEST_ASSERT_EQUAL(CLOCK_SOURCE_USB, manager.getActiveSource());
    
    // SYNC_IN takes over
    TEST_ASSERT_TRUE(manager.acceptClock(CLOCK_SOURCE_SYNC_IN));
    TEST_ASSERT_EQUAL(CLOCK_SOURCE_SYNC_IN, manager.getActiveSource());
    
    // USB should be rejected while SYNC_IN is active
    TEST_ASSERT_FALSE(manager.acceptClock(CLOCK_SOURCE_USB));
    TEST_ASSERT_EQUAL(CLOCK_SOURCE_SYNC_IN, manager.getActiveSource());
}

// Test priority: SYNC_IN blocks DIN
void test_priority_sync_in_blocks_din() {
    manager.reset();
    
    // Start with DIN
    TEST_ASSERT_TRUE(manager.acceptClock(CLOCK_SOURCE_DIN));
    TEST_ASSERT_EQUAL(CLOCK_SOURCE_DIN, manager.getActiveSource());
    
    // SYNC_IN takes over
    TEST_ASSERT_TRUE(manager.acceptClock(CLOCK_SOURCE_SYNC_IN));
    TEST_ASSERT_EQUAL(CLOCK_SOURCE_SYNC_IN, manager.getActiveSource());
    
    // DIN should be rejected while SYNC_IN is active
    TEST_ASSERT_FALSE(manager.acceptClock(CLOCK_SOURCE_DIN));
    TEST_ASSERT_EQUAL(CLOCK_SOURCE_SYNC_IN, manager.getActiveSource());
}

// Test priority: USB blocks DIN
void test_priority_usb_blocks_din() {
    manager.reset();
    
    // Start with DIN
    TEST_ASSERT_TRUE(manager.acceptClock(CLOCK_SOURCE_DIN));
    TEST_ASSERT_EQUAL(CLOCK_SOURCE_DIN, manager.getActiveSource());
    
    // USB takes over
    TEST_ASSERT_TRUE(manager.acceptClock(CLOCK_SOURCE_USB));
    TEST_ASSERT_EQUAL(CLOCK_SOURCE_USB, manager.getActiveSource());
    
    // DIN should be rejected while USB is active
    TEST_ASSERT_FALSE(manager.acceptClock(CLOCK_SOURCE_DIN));
    TEST_ASSERT_EQUAL(CLOCK_SOURCE_USB, manager.getActiveSource());
}

// Test fallback: When SYNC_IN stops, USB can take over
void test_fallback_sync_in_to_usb() {
    manager.reset();
    
    // SYNC_IN is active
    TEST_ASSERT_TRUE(manager.acceptClock(CLOCK_SOURCE_SYNC_IN));
    TEST_ASSERT_EQUAL(CLOCK_SOURCE_SYNC_IN, manager.getActiveSource());
    
    // Stop SYNC_IN
    manager.stop(CLOCK_SOURCE_SYNC_IN);
    
    // USB should now be accepted
    TEST_ASSERT_TRUE(manager.acceptClock(CLOCK_SOURCE_USB));
    TEST_ASSERT_EQUAL(CLOCK_SOURCE_USB, manager.getActiveSource());
}

// Test fallback: When USB stops, DIN can take over
void test_fallback_usb_to_din() {
    manager.reset();
    
    // USB is active
    TEST_ASSERT_TRUE(manager.acceptClock(CLOCK_SOURCE_USB));
    TEST_ASSERT_EQUAL(CLOCK_SOURCE_USB, manager.getActiveSource());
    
    // Stop USB
    manager.stop(CLOCK_SOURCE_USB);
    
    // DIN should now be accepted
    TEST_ASSERT_TRUE(manager.acceptClock(CLOCK_SOURCE_DIN));
    TEST_ASSERT_EQUAL(CLOCK_SOURCE_DIN, manager.getActiveSource());
}

// Test complete priority chain
void test_priority_chain_full() {
    manager.reset();
    
    // Start with lowest priority (DIN)
    TEST_ASSERT_TRUE(manager.acceptClock(CLOCK_SOURCE_DIN));
    TEST_ASSERT_EQUAL(CLOCK_SOURCE_DIN, manager.getActiveSource());
    
    // USB takes over
    TEST_ASSERT_TRUE(manager.acceptClock(CLOCK_SOURCE_USB));
    TEST_ASSERT_EQUAL(CLOCK_SOURCE_USB, manager.getActiveSource());
    
    // SYNC_IN takes over (highest priority)
    TEST_ASSERT_TRUE(manager.acceptClock(CLOCK_SOURCE_SYNC_IN));
    TEST_ASSERT_EQUAL(CLOCK_SOURCE_SYNC_IN, manager.getActiveSource());
    
    // Both USB and DIN should be rejected
    TEST_ASSERT_FALSE(manager.acceptClock(CLOCK_SOURCE_USB));
    TEST_ASSERT_FALSE(manager.acceptClock(CLOCK_SOURCE_DIN));
    TEST_ASSERT_EQUAL(CLOCK_SOURCE_SYNC_IN, manager.getActiveSource());
}

// Test initial state
void test_initial_state_accepts_any_source() {
    manager.reset();
    
    TEST_ASSERT_EQUAL(CLOCK_SOURCE_NONE, manager.getActiveSource());
    
    // Any source should be accepted from idle state
    TEST_ASSERT_TRUE(manager.acceptClock(CLOCK_SOURCE_DIN));
    
    manager.reset();
    TEST_ASSERT_TRUE(manager.acceptClock(CLOCK_SOURCE_USB));
    
    manager.reset();
    TEST_ASSERT_TRUE(manager.acceptClock(CLOCK_SOURCE_SYNC_IN));
}

void setUp(void) {
    // Reset manager before each test
    manager.reset();
}

void tearDown(void) {
    // Clean up after each test
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    // Priority blocking tests
    RUN_TEST(test_priority_sync_in_blocks_usb);
    RUN_TEST(test_priority_sync_in_blocks_din);
    RUN_TEST(test_priority_usb_blocks_din);
    
    // Fallback tests
    RUN_TEST(test_fallback_sync_in_to_usb);
    RUN_TEST(test_fallback_usb_to_din);
    
    // Integration tests
    RUN_TEST(test_priority_chain_full);
    RUN_TEST(test_initial_state_accepts_any_source);
    
    return UNITY_END();
}
