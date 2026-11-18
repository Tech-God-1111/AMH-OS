"""Restricted hardware API. Verify the exact board pinout before enabling pins."""

ALLOWED_OUTPUT_PINS = ()


def set_output(pin_number, value):
    pin_number = int(pin_number)
    if pin_number not in ALLOWED_OUTPUT_PINS:
        raise ValueError("GPIO output disabled until board pinout is verified")
    from machine import Pin
    pin = Pin(pin_number, Pin.OUT)
    pin.value(1 if value else 0)
    return {"pin": pin_number, "value": pin.value()}
