

   MEMBER('loader.clw')                               ! This is a MEMBER module


   INCLUDE('ABTOOLBA.INC'),ONCE
   INCLUDE('ABWINDOW.INC'),ONCE

                     MAP
                       INCLUDE('LOADE002.INC'),ONCE        !Local module prodecure declarations
                     END


about PROCEDURE                                       !Generated from procedure template - Window

FilesOpened          BYTE
window               WINDOW('About'),AT(,,255,78),FONT('Arial',8,,,CHARSET:ANSI),SYSTEM,GRAY,MODAL
                       IMAGE('logo.jpg'),AT(12,12,99,32),USE(?logo)
                       STRING('LPC3250 NOR Flash Loader'),AT(128,16),USE(?String1),FONT(,,,FONT:bold)
                       STRING('version  0.1'),AT(142,31),USE(?String2)
                       LINE,AT(6,47,237,0),USE(?Line1),COLOR(COLOR:Black)
                       STRING('Copyright NXP 2009 - All Rights reserved'),AT(16,56),USE(?String3)
                       BUTTON('Ok'),AT(176,55,45,14),USE(?ok)
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
  GlobalErrors.SetProcedureName('about')
  SELF.Request = GlobalRequest
  ReturnValue =PARENT.Init()
  IF ReturnValue THEN RETURN ReturnValue.
  SELF.FirstField = ?logo
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
  ReturnValue =PARENT.TakeAccepted()
    CASE ACCEPTED()
    OF ?ok
      ThisWindow.Update
      POST(Event:CloseWindow)
    END
    RETURN ReturnValue
  END
  ReturnValue = Level:Fatal
  RETURN ReturnValue

