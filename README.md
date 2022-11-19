# RAK4631 RAK1906 BSEC

An example project demonstrating how to save the RAK1906/BME680's IAQ state to the File System via LittleFS and retrieve it when rebooting, in order to avoid restarting the 4-day heat-up period.

It is using the `config/generic_33v_3s_4d/bsec_iaq.txt` config file provided by Bosch. I believe that the IAQ polling frequency should then be 3 seconds – but Bosch is being a bit sparse on details. In the sketch the IAQ polling delay is 10 seconds, BUT,... you can adjust that in the Serial Monitor.

## Batteries Included!

This sketch comes with an extensible command framework, which has for now 5 commands:

```
/help
Available commands: 5
 . help: Shows this help.
 . bsec_fq: Gets/sets the save interval in seconds.
 . iaq_fq: Gets/sets the IAQ polling in seconds.
 . save: Saves BSEC status.
 . poll: Polls the BME680.
```

You can enquire at which frequency the IAQ is polled, as I mentioned, and at which frequency the IAQ status is saved to the file system. You don't want to do this too often. It is set up right now at one hour, but should probably be set at something longer, 4 hours or more.

You can also force-save, if you so desire – this can be useful if you need to unplug, making sure you have the latest before turning off. And of course force-polling the BME680 has no particular side effects. If you're impatient, hit that `/save` command!


```
Let's go!

BSEC library version 1.4.8.0
init_flash
loadState:
   +------------------------------------------------+ +----------------+
   |.0 .1 .2 .3 .4 .5 .6 .7 .8 .9 .a .b .c .d .e .f | |      ASCII     |
   +------------------------------------------------+ +----------------+
 0.|00 08 04 01 3d 00 00 00 00 00 00 00 73 00 00 00 | |....=.......s...|
 1.|2d 00 01 a5 00 00 00 00 00 00 00 00 00 00 00 00 | |-...............|
 2.|00 00 00 00 00 00 00 00 ff ff 02 02 00 00 00 00 | |................|
 3.|00 00 00 00 00 00 00 00 00 00 00 00 02 0c 00 02 | |................|
 4.|a5 00 00 c8 41 00 00 c8 41 10 00 03 a5 00 00 00 | |....A...A.......|
 5.|00 00 00 00 00 00 00 00 00 16 00 05 a5 00 00 00 | |................|
 6.|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 0c | |................|
 7.|00 09 a5 ff ff ff ff ff ff ff 7f 08 00 0a a5 00 | |................|
   +------------------------------------------------+ +----------------+
 8.|00 00 00 00 00 00 00 ca 96 00 00                | |...........     |
   +------------------------------------------------+ +----------------+

Timestamp: 13.06 secs
 . Raw Temperature: 33.97 C
 . Pressure: 999.74 HPa
 . Raw Humidity: 50.98%
 . Gas Resistance: 0
 . IAQ: 25.00
 . IAQ Accuracy: 0.00
 . Temperature: 33.97 C
 . Humidity: 50.98%
 . Static IAQ: 25.00
 . CO2 Equivalent: 500.00
 . Breath VOC EquivalentQ: 0.50

updateState
Writing state to Flash... done!
   +------------------------------------------------+ +----------------+
   |.0 .1 .2 .3 .4 .5 .6 .7 .8 .9 .a .b .c .d .e .f | |      ASCII     |
   +------------------------------------------------+ +----------------+
 0.|00 08 04 01 3d 00 00 00 00 00 00 00 73 00 00 00 | |....=.......s...|
 1.|2d 00 01 a5 00 00 00 00 00 00 00 00 00 00 00 00 | |-...............|
 2.|00 00 00 00 00 00 00 00 2c 01 14 14 00 00 00 00 | |........,.......|
 3.|00 00 00 00 00 00 03 00 17 a8 5f 3f 14 0c 00 02 | |.........._?....|
 4.|a5 00 00 c8 41 00 00 c8 41 10 00 03 a5 5a 48 7b | |....A...A....ZH{|
 5.|40 47 e1 07 42 73 e8 4b 42 16 00 05 a5 c0 74 e6 | |@G..Bs.KB.....t.|
 6.|09 03 00 00 00 00 00 00 00 00 00 00 00 01 00 0c | |................|
 7.|00 09 a5 c0 74 e6 09 03 00 00 00 08 00 0a a5 47 | |....t..........G|
   +------------------------------------------------+ +----------------+
 8.|e1 07 42 00 00 00 00 2b c0 00 00                | |..B....+...     |
   +------------------------------------------------+ +----------------+
/iaq_fq
Polling Interval: every 10 s
/bsec_fq
Save Interval: every 3600 s
/iaq_fq 30
* Polling Interval set to 30 s
/bsec_fq 4800
* Save interval set to 4800 s
/poll
Force-polling the BME680:
Timestamp: 43.30 secs
 . Raw Temperature: 34.04 C
 . Pressure: 999.70 HPa
 . Raw Humidity: 51.01%
 . Gas Resistance: 0
 . IAQ: 25.00
 . IAQ Accuracy: 0.00
 . Temperature: 34.03 C
 . Humidity: 50.89%
 . Static IAQ: 25.00
 . CO2 Equivalent: 500.00
 . Breath VOC EquivalentQ: 0.50
```

*Now waiting desperately for that ` . IAQ: 25.00` to change to something meaningful...*