# pwmfan

|func	|atmega328p	|arduino
|-------|---------------|------
|PWM0	|PB0		|D8
|PWM1	|PB1		|D9
|PWM2	|PB2		|D10
|PWM3	|PB3		|D11
|PWM4	|PB4		|D12
|PWM5	|PB5		|D13
|RPM0	|PC0		|A0
|RPM1	|PC1		|A1
|RPM2	|PC2		|A2
|RPM3	|PC3		|A3
|RPM4	|PC4		|A4
|RPM5	|PC5		|A5
	

# 115200 8n1
```
Set PWM in 0..255 <channel>:<value>,

Example:
>0:100,1:200,2:0,3:255,4:0,5:0,

Answer to any request is: Get RPM
<2000,2500,0,2800,0,0,
```
