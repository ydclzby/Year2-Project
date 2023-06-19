from machine import Pin, ADC, PWM

vret_pin = ADC(Pin(26))
vout_pin = ADC(Pin(28))
vin_pin = ADC(Pin(27))
pwm = PWM(Pin(0))
pwm.freq(100000)
pwm_en = Pin(1, Pin.OUT)

count = 0
pwm_out = 0
pwm_ref = 0
setpoint = 0.0
delta = 0.05

v_ref = 2.2

def saturate(duty):
    if duty > 62500:
        duty = 62500
    if duty < 100:
        duty = 100
    return duty

while True:
    
    pwm_en.value(1)

    vin = vin_pin.read_u16()
    vout = vout_pin.read_u16()
    vret = vret_pin.read_u16()
    count = count + 1
    
    Vo = (vout / 2.5) * 10
    Vo = Vo - vret
    Vo = Vo * 4 / 62500
    Io = Vo / 2.2
    
    if Vo < v_ref:
        pwm_ref = pwm_ref + 1
    if Vo > v_ref:
        pwm_ref = pwm_ref - 1

    pwm_out = saturate(pwm_ref)
    pwm.duty_u16(pwm_out)
    
    duty_cycle = pwm_out / 62500
    
    if count > 2000:
        print("Vo = ",Vo)
        print("Io = ",Io)
        print("DutyCycle = ",duty_cycle)
        
        count = 0