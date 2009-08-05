"""
pyWUtils

Utility classes and functions for Python.

by Wesley R. Elsberry

"""

import sys
import string

def linmap(dy,dl,dh,rl,rh):
    """
    linmap

    Function that does linear interpolation and extrapolation.  Given
    a data value (dy), a domain low value (dl), a domain high value
    (dh), a range low value (rl), a range high value (rh), this
    returns the value in the range that corresponds to the data value
    in the domain.
    """
    if (dl != dh) and (rl != rh):
        ry = (dy - dl) * (rh - rl) / (dh - dl)
        return(ry)
    else: 
        return(0.0)

class wstring(str):
    """
    wstring

    Class to extend Python str for common text-processing methods
    useful in parsing text input.
    """

    def __init__(self,instr):
        self.value = instr

    def before(self,pat):
        """
        before

        Returns the part of the string that occurs before the first
        appearance of pat, or the complete string.
        """
        idx = self.find(pat)
        if (-1 == idx):
            return(self.value)
        else:
            return(self.value[:idx])

    def after(self,pat):
        """
        after

        Returns the part of the string that follows pat, or the null
        string.
        """
        idx = self.find(pat)
        if (-1 == idx):
            return("")
        else:
            return(self.value[idx + len(pat):])

    def psplit(self,pat):
        """
        psplit

        Returns a tuple with two strings, the 
        """
        b = self.before(pat)
        a = self.after(pat)
        return((b,a,))

class lreg():
    """
    lreg

    Class for handling linear regression, especially for incorporating into
    processes that incrementally add data points.
    """

    def __init__(self):
        self.sumx = 0.0
        self.sumy = 0.0
        self.n = 0
        self.sumxy = 0.0
        self.sumx2 = 0.0
        self.sumy2 = 0.0
        self.r = 0.0
        self.r2 = 0.0
        self.avgx = 0.0
        self.avgy = 0.0

    def addXY(self,x,y):
        if (None != x) and (None != y):
            self.n += 1
            self.sumx += x
            self.sumy += y
            self.sumx2 += x * x
            self.sumy2 += y * y
            self.sumxy += x * y
            self.avgx = self.sumx / self.n
            self.avgy = self.sumy / self.n
            
    



