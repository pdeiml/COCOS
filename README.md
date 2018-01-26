# COCOS
Programm zur Analyse unserer Messdaten.



Aktueller Stand der Dinge:

-> Kalibriermöglichkeiten: Neben dem einfachen Auswerten gibt es auch die Möglichkeit, eine Kalibrationsdatei zu erstellen ("Calibration.txt") oder die Kalibrationsdatei einzulesen und anzuwenden. Die Anwendung ist nicht idiotensicher, es ist darauf zu achten, dass bei der Erstellung der Calibration.txt die gleichen output-Einstellungen (starttime, endtime, Anzahl bins) gewählt wurden wie bei der Anwendung der Kalibration. Die Kalibration wird bei entsprechender Einstellung direkt in die g2-Funktion implementiert.

-> FFT nun von der g2-Funktion und nicht mehr von den Events, so dass der Effekt angewandter Kalibration in der FFT sichtbar wird.


# COCOShpc
HPC-Version (ohne Interaktion). Die Einstellungen müssen hierzu in der 'settings.txt' Datein vorgenommen werden, im Format, wie sie in der Beispiel-Datei mitgeliefert wird.
