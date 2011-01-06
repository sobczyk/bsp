

   MEMBER('loader.clw')                               ! This is a MEMBER module


   INCLUDE('ABTOOLBA.INC'),ONCE
   INCLUDE('ABWINDOW.INC'),ONCE

                     MAP
                       INCLUDE('LOADE003.INC'),ONCE        !Local module prodecure declarations
                     END


setup PROCEDURE                                       !Generated from procedure template - Window

FilesOpened          BYTE
window               WINDOW('Configure Port'),AT(,,106,56),FONT('Arial',8,,,CHARSET:ANSI),GRAY,MODAL
                       PROMPT('Port:'),AT(16,12),USE(?com:Prompt)
                       LIST,AT(40,12,48,10),USE(com),VSCROLL,DROP(5),FROM('COM1|COM2|COM3|COM4|COM5|COM6|COM7|COM8')
                       BUTTON('Continue...'),AT(16,32,69,14),USE(?Button1)
                     END

ThisWindow           CLASS(WindowManager)
Init                   PROCEDURE(),BYTE,PROC,DERIVED
Kill                   PROCEDURE(),BYTE,PROC,DERIVED
TakeAccepted           PROCEDURE(),BYTE,PROC,DERIVED
                     END

Toolbar              ToolbarClass

  CODE
  GlobalResponse = ThisWindow.Run()


ThisWindow.Init PROCEDURE

ReturnValue          BYTE,AUTO
  CODE
  com = GETINI('Configuration','COM','COM1','.\Loader.INI')
  port = GETINI('Configuration','PORT',0,'.\Loader.INI')
  GlobalErrors.SetProcedureName('setup')
  SELF.Request = GlobalRequest
  ReturnValue =PARENT.Init()
  IF ReturnValue THEN RETURN ReturnValue.
  SELF.FirstField = ?com:Prompt
  SELF.VCRRequest &= VCRRequest
  SELF.Errors &= GlobalErrors
  SELF.AddItem(Toolbar)
  CLEAR(GlobalRequest)
  CLEAR(GlobalResponse)
  OPEN(window)
  SELF.Opened=True
  SELF.SetAlerts()
  RETURN ReturnValue


ThisWindow.Kill PROCEDURE

ReturnValue          BYTE,AUTO
  CODE
  ReturnValue =PARENT.Kill()
  IF ReturnValue THEN RETURN ReturnValue.
  GlobalErrors.SetProcedureName
  RETURN ReturnValue


ThisWindow.TakeAccepted PROCEDURE

ReturnValue          BYTE,AUTO
Looped BYTE
  CODE
  LOOP
    IF Looped
      RETURN Level:Notify
    ELSE
      Looped = 1
    END
    CASE ACCEPTED()
    OF ?Button1
      PUTINI('Configuration','COM',com,'.\Loader.INI')
      CASE com
      OF 'COM1'
        port=0
      OF 'COM2'
        port=1
      OF 'COM3'
        port=2
      OF 'COM4'
        port=3
      OF 'COM5'
        port=4
      OF 'COM6'
        port=5
      OF 'COM7'
        port=6
      OF 'COM8'
        port=7
      END
      PUTINI('Configuration','PORT',port,'.\Loader.INI')
      POST(Event:CloseWindow)
    END
  ReturnValue =PARENT.TakeAccepted()
    RETURN ReturnValue
  END
  ReturnValue = Level:Fatal
  RETURN ReturnValue

