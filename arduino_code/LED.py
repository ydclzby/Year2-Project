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

i_ref = 0.2 #current reference

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
    
    V_ret = vret * 3.3 / 65536 #digital to analogue conversion
    Io = V_ret / 1.02          #current output = vret divided by total resistance of five parallel 5.1Ω resistances
    
    if Io < i_ref:
        pwm_ref = pwm_ref + 1 #voltage increases
    if Io > i_ref:
        pwm_ref = pwm_ref - 1 #voltage decreases

    pwm_out = saturate(pwm_ref)
    pwm.duty_u16(pwm_out)
    dc = pwm_out / 65536
    
    if count > 2000:
        print("Vo = ",Vo)
        print("Io = ",Io)
        print("Duty = ",dc)
        
        count = 0
