import micPython as mic
import matplotlib.pyplot as plt
import numpy as np
from scipy.fft import fft, fftfreq
import scipy.signal as sgn

print(mic.enumdevices())
print(mic.initialize())
mic.opendevice()
dati=mic.rec()
mic.close()
filtropb=sgn.iirdesign(40/mic.fCampionamento, 20/mic.fCampionamento, 1, 180, ftype='butter', output='sos', fs=mic.fCampionamento)
segnalefiltrato=sgn.sosfilt(filtropb, dati)

NumerodiCampioni=len(dati)
fourierAudio=fft(segnalefiltrato)
xf=fftfreq(NumerodiCampioni, 1/mic.fCampionamento)[:NumerodiCampioni // 2]
fig, (ax1, ax2)=plt.subplots(2, 1)
ax1.plot(segnalefiltrato, 'r.-')
ax2.plot(xf, 2.0/NumerodiCampioni * np.abs(fourierAudio[0:NumerodiCampioni//2]))
plt.show()

mic.release()
