import micPython as mic
import numpy as np
import matplotlib.pyplot as plt
from scipy.fft import fft, fftfreq

if __name__ == '__main__': #se il modulo python è eseguito da riga di comando
    if mic.initialize():
        print(mic.enumdevices()) #vedo quali dispositivi in input ho
        mic.opendevice() #apro quello di default (0)
        campioniMicrofono=mic.rec(2.0)
        mic.close()
    
        
        #spettro delle frequenze
        numerodiCampioni = len(campioniMicrofono)
        spettro=fft(campioniMicrofono)
        xf=fftfreq(numerodiCampioni, 1/mic.fCampionamento)[:numerodiCampioni // 2]
        
        #plot dei campioni e dello spettro
        fig,(ax1,ax2) = plt.subplots(2,1)
        ax1.plot(np.linspace(0.0,1.0,numerodiCampioni,endpoint=False,),campioniMicrofono, 'b.')
        ax2.plot(xf[20:len(xf)], 2.0/numerodiCampioni * np.abs(spettro[20:numerodiCampioni // 2]), 'r-')
        plt.show()

        mic.release()
