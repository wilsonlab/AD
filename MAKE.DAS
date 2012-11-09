#
# Borland C++ IDE generated makefile
#
.AUTODEPEND


#
# Borland C++ tools
#
IMPLIB  = Implib
BCCDOS  = Bcc +BccDos.cfg 
TLINK   = TLink
TLIB    = TLib
TASM    = Tasm
#
# IDE macros
#


#
# Options
#
IDE_LFLAGSDOS =  -LE:\BC4\LIB
IDE_BFLAGS = 
LLATDOS_addexe =  -c -Tde -LC:\BC45\LIB
RLATDOS_addexe = 
BLATDOS_addexe = 
CNIEAT_addexe = -IC:\BC45\INCLUDE;C:\BC45\SVGA -D
LNIEAT_addexe = -x
LEAT_addexe = $(LLATDOS_addexe)
REAT_addexe = $(RLATDOS_addexe)
BEAT_addexe = $(BLATDOS_addexe)
CLATDOS_addc =  -Ot -Oc -v- -R- -k- -f -ff -Om -Op -Oi -Ov
LLATDOS_addc = 
RLATDOS_addc = 
BLATDOS_addc = 
CEAT_addc = $(CEAT_addexe) $(CLATDOS_addc)
CNIEAT_addc = -IC:\BC45\INCLUDE;C:\BC45\SVGA -D
LNIEAT_addc = -x
LEAT_addc = $(LEAT_addexe) $(LLATDOS_addc)
REAT_addc = $(REAT_addexe) $(RLATDOS_addc)
BEAT_addc = $(BEAT_addexe) $(BLATDOS_addc)

#
# Dependency List
#
Dep_ad = \
   ad.exe

ad : BccDos.cfg $(Dep_ad)
  echo MakeNode 

Dep_addexe = \
   9513.obj\
   adtrack.obj\
   adio.obj\
   addisp.obj\
   combus2.obj\
   addma.obj\
   adseq.obj\
   adamp.obj\
   adpio2.obj\
   das1802.obj\
   ad.obj

ad.exe : $(Dep_addexe)
  $(TLINK)   @&&|
 /v $(IDE_LFLAGSDOS) $(LEAT_addexe) $(LNIEAT_addexe) +
C:\BC45\LIB\c0l.obj+
9513.obj+
adamp.obj+
adpio2.obj+
adio.obj+
adtrack.obj+
addisp.obj+
addma.obj+
combus2.obj+
adseq.obj+
das1802.obj+
ad.obj
$<,$*
C:\BC45\LIB\graphics.lib+
C:\BC45\LIB\fp87.lib+
C:\BC45\LIB\mathl.lib+
C:\BC45\LIB\cl.lib

|

9513.obj :  9513.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_addexe) $(CNIEAT_addexe) -o$@ 9513.c
|

adio.obj :  adio.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_addexe) $(CNIEAT_addexe) -o$@ adio.c
|

addisp.obj :  addisp.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_addexe) $(CNIEAT_addexe) -o$@ addisp.c
|

addma.obj :  addma.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_addexe) $(CNIEAT_addexe) -o$@ addma.c
|

combus2.obj :  combus2.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_addexe) $(CNIEAT_addexe) -o$@ combus2.c
|

adseq.obj :  adseq.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_addexe) $(CNIEAT_addexe) -o$@ adseq.c
|

adamp.obj :  adamp.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_addexe) $(CNIEAT_addexe) -o$@ adamp.c
|

adpio2.obj :  adpio2.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_addexe) $(CNIEAT_addexe) -o$@ adpio2.c
|

adtrack.obj :  adtrack.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_addexe) $(CNIEAT_addexe) -o$@ adtrack.c
|

das1802.obj :  das1802.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_addexe) $(CNIEAT_addexe) -o$@ das1802.c
|

ad.obj :  ad.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_addc) $(CNIEAT_addc) -o$@ ad.c
|

# Compiler configuration file
BccDos.cfg : 
   Copy &&|
-W-
-R
-v
-vi
-H
-H=grexam.csm
-X-
-Om
-Op
-Oi
-Ov
-Og
-Ot
-v-
-R-
-k-
-wlin
-ml
-f287
-Z
-O
-Oe
-Ol
-Ob
-OW
-O-a
| $@


