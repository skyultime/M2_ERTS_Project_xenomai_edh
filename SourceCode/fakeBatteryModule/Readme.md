From hoelzro : https://github.com/hoelzro/linux-fake-battery-module

License : GPL-2.0

From hoelzro : https://github.com/hoelzro/linux-fake-battery-module

License : GPL-2.0

Example :

$ echo 'charging = 0' | sudo tee /dev/fake_battery # set state to discharging
$ echo 'charging = 1' | sudo tee /dev/fake_battery # set state to charging
$ echo 'capacity0 = 77' | sudo tee /dev/fake_battery # set charge on BAT0 to 77%
$ echo 'capacity1 = 77' | sudo tee /dev/fake_battery # set charge on BAT1 to 77%
