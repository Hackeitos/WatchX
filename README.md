# WatchX
(LS = Left Switch, URS = Upper Right Switch, LRS = Lower Right Switch)

When you first upload the sketch, if the leds turn on for a moment, it means that the rtc has lost power and the time was set to the moment the sketch was compiled, next it will appear the time screen. Upper right is the temperature in C, upper left, the battery percentage ([TODO] it does not show when it is charging), lower middle is the date and in the center appears the current time. At this point if you press both the LS and URS you will access to settings, explained below (LS to next setting, URS to raise value, LRS to decrease value):

- Second (Int): The current second

- Minute (Int): The current minute

- Hour (Int): The current hour

- Day (Int): The current month day

- Month (Int): The current month

- Year (Int): The current year

(For your knowledge, the time and date wont be applied until you press LS in the year setting)

- X Axis (Float): When the accelerometer reads reach this value (+- Sensibility) the watch will be turned on

- Sensibility (Float): The range the X Axis allows for turning on watch (if 0, accel is disabled)

- Calibrated compass (Bool): If 0, you can press any the URS or LRS and wait for it to calibrate

- Temperature (Float): Due I have seen that the temperature given by the watch is incorrect, this number will beadded to the raw temperature reading (negative values will work too)

- Timeout (Int): The time (in seconds) the watch will be kept its screen on (If 0 it wont turn off never, unless out ofbattery)

- Dim (Bool): Wether to lower the screen brightness or not

Compass:

Once you changed the settings to the values you like, you will return to the time screen, and if you press the LRB the compass value and your facing will appear next to the temperature (It is toggleable, so you can disable it by pressing again). If not calibrated, instead yo will see "CAL".

Stopwatch & Countdown:

Still in time screen, you can press the LS and you will go to the StopWatch/CountDown screen (maybe wit a fast press it wont realize you pressed it). A "S" appears under the battery value if it is in stopwatch mode and a "C" if it is in countdown mode. Now, in this screen, if you press the URS the SW/CD will reset and pressing the LRS will start the SW/CD (the screen wont update until you release the button, but it starts counting at the moment you press it). Now, how to switch between SW and CD? Simple, in this screen if you press both the LS and URS, you will access to the CD settings where, if you set all values to 0, it will become a stopwatch (default). If in CD mode, when it reaches 0 the leds will blink until you press sny switch (has no sound yet). If in SW mode, while the minutes < 60 it will show mm:ss:(10th of a second), else it will show HH:mm:ss. The countdown wont blink leds if not in countdown screen [TODO], but it will be still counting (stopwatch too).

IMPORTANT: The watch wont turn off if compass or stopwatch/contdown are shown for comfort issues. You can change this in line 81 erasing the "&& !timer && !compass".

One of the problems is the battery, if you want more battery life you can increase the delays at lines 73 and 86 (Do not raise it above 1000, it will explode).

You are completely free to improve(I say improve becouse it is impossible to worsen) this code and post it here or anywhere, but please, credit me (u/Hackeitos). You are free to suggest anything too.

If you saw this and now your eyes are bleeding, I'm spanish and my english level is low, so you can ask me for new ones.

Thanks so much to the WatchX team to make this awesome watch possible.

# TODOs:
  - Battery gets over so fast, improve power saving
  - Countdown doesn't blink when on time screen, fix that
  - Make the charging indicator appear
