   PROGRAM

_ABCDllMode_  EQUATE(0)
_ABCLinkMode_ EQUATE(1)

   INCLUDE('ABERROR.INC '),ONCE
   INCLUDE('ABFILE.INC  '),ONCE
   INCLUDE('ABUTIL.INC  '),ONCE
   INCLUDE('ABWINDOW.INC'),ONCE
   INCLUDE('EQUATES.CLW'),ONCE
   INCLUDE('ERRORS.CLW'),ONCE
   INCLUDE('KEYCODES.CLW'),ONCE

   MAP
     MODULE('LOADEBC.CLW')
DctInit     PROCEDURE
DctKill     PROCEDURE
     END
!--- Application Global and Exported Procedure Definitions --------------------------------------------
     MODULE('LOADE001.CLW')
Main                   PROCEDURE   !
     END
       Include('CLACOM.INC')
   END

startAddress       STRING('0xE0000000')
hwin               UNSIGNED
retcode            SHORT
char               BYTE
EOF                BYTE
index              LONG
state              SHORT
size               LONG
val_word           BYTE(1)
val_word_sent      BYTE
filename           STRING(120)
dir                STRING(120)
port               SHORT
com                STRING(5)
list               QUEUE,PRE(q)
line                 STRING(120)
                   END
n                  LONG

bin                  FILE,DRIVER('DOS'),NAME(filename),PRE(BIN),CREATE,BINDABLE,THREAD                    
Record                   RECORD,PRE()
data                        BYTE
                         END
                     END                       

loader               FILE,DRIVER('DOS'),NAME('loader.dat'),PRE(LOA),BINDABLE,THREAD                 
Record                   RECORD,PRE()
data                        BYTE
                         END
                     END                       




! CLACom Data and Equates
!
Include('CLACOMST.INC')
Access:bin           &FileManager
Relate:bin           &RelationManager
Access:loader        &FileManager
Relate:loader        &RelationManager
GlobalErrors         ErrorClass
INIMgr               INIClass
GlobalRequest        BYTE(0),THREAD
GlobalResponse       BYTE(0),THREAD
VCRRequest           LONG(0),THREAD

  CODE
  GlobalErrors.Init
  INIMgr.Init('loader.INI')
  DctInit
  Main
  INIMgr.Update
  INIMgr.Kill
  DctKill
  GlobalErrors.Kill





