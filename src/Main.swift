let BUTTON_PINS: [UInt16] = [0, 1, 2, 3, 4, 5, 6, 7]
let LED_PINS: [UInt16] = [8, 9, 10, 11, 12, 13, 14, 15]

let DEBOUNCE_DELAY_MS: UInt16 = 5

let INTERVAL_MS: UInt16 = 8

let KEYS: [UInt8] = [0x4, 0x16, 0x7, 0xf, 0x33, 0x6, 0x5, 0x28]

var start_ms: UInt32 = 0
var remote_wakeup_enabled: Bool = false
var last_button_state: [Bool] = [
    false, false, false, false, false, false, false, false,
]
var last_button_press_ms: [UInt32] = [0, 0, 0, 0, 0, 0, 0, 0]

@main
struct Main {
    static func main() {
        board_init()
        tusb_init()

        for pin in BUTTON_PINS {
            let pin32 = UInt32(pin)
            gpio_init(pin32)
            gpio_set_dir(pin32, false)
            gpio_pull_up(pin32)
        }

        for pin in LED_PINS {
            let pin32 = UInt32(pin)
            gpio_init(pin32)
            gpio_set_dir(pin32, true)
        }

        while true {
            tud_task()
            hid_task()
        }
    }
}

@_cdecl("tud_mount_cb")
func tud_mount_cb() {}

@_cdecl("tud_umount_cb")
func tud_umount_cb() {}

@_cdecl("tud_suspend_cb")
func tud_suspend_cb(remote_wakeup_en: Bool) {
    remote_wakeup_enabled = remote_wakeup_en
}

@_cdecl("tud_resume_cb")
func tud_resume_cb() {}

func sendKeys() {
    guard tud_hid_ready() else { return }

    var report = [UInt8](repeating: 0, count: 10)
    var last_idx = 2

    for (pin, state) in last_button_state.enumerated() {
        if state {  // button is pressed
            let key = KEYS[pin]
            report[last_idx] = key
            last_idx += 1
        }
        gpio_put(UInt32(LED_PINS[pin]), state)
    }
    tud_hid_report(1 /*REPORT_ID_KEYBOARD*/, report, 8)
}

func hid_task() {
    guard to_ms_since_boot(get_absolute_time()) - start_ms > UInt32(INTERVAL_MS) else { return }

    start_ms += UInt32(INTERVAL_MS)
    for pin in BUTTON_PINS {
        let pinInt = Int(pin)
        guard
            to_ms_since_boot(get_absolute_time()) - last_button_press_ms[pinInt]
                > UInt32(DEBOUNCE_DELAY_MS)
        else { continue }
        let state = !gpio_get(UInt32(pin))
        if state != last_button_state[pinInt] {
            last_button_state[pinInt] = state
            last_button_press_ms[pinInt] = to_ms_since_boot(get_absolute_time())
        }
    }

    sendKeys()
}

@_cdecl("tud_hid_report_complete_cb")
func tud_hid_report_complete_cb(instance: UInt8, report: UnsafePointer<UInt8>, len: UInt16) {}

@_cdecl("tud_hid_get_report_cb")
func tud_hid_get_report_cb(
    instance: UInt8, report_id: UInt8, report_type: UnsafePointer<hid_report_type_t>,
    buffer: UnsafeMutablePointer<UInt8>, len: UInt16
) -> UInt16 {
    // nothing to do
    return 0
}

@_cdecl("tud_hid_set_report_cb")
func tud_hid_set_report_cb(
    instance: UInt8, report_id: UInt8, report_type: UnsafePointer<hid_report_type_t>,
    buffer: UnsafePointer<UInt8>, len: UInt16
) {}
