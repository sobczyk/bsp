   ! COM Port Equates

COM1        EQUATE(0)
COM2        EQUATE(1)
COM3        EQUATE(2)
COM4        EQUATE(3)
COM5        EQUATE(4)
COM6        EQUATE(5)
COM7        EQUATE(6)
COM8        EQUATE(7)

NOPARITY    EQUATE(0)
ODDPARITY   EQUATE(1)
EVENPARITY  EQUATE(2)

ONESTOPBIT  EQUATE(1)
TWOSTOPBIT  EQUATE(2)

USECTS      EQUATE(1)
NOCTS       EQUATE(0)
USEXON      EQUATE(1)
NOXON       EQUATE(0)
USEDSR      EQUATE(1)
NODSR       EQUATE(0)

DTRON       EQUATE(1)
DTROFF      EQUATE(0)
RTSON       EQUATE(1)
RTSOFF      EQUATE(0)

BLACK       EQUATE(0)
BLUE        EQUATE(1)
GREEN       EQUATE(2)
CYAN        EQUATE(3)
RED         EQUATE(4)
MAGENTA     EQUATE(5)
BROWN       EQUATE(6)
LTGREY      EQUATE(7)
DKGRAY      EQUATE(8)
LTBLUE      EQUATE(9)
LTGREEN     EQUATE(10)
LTCYAN      EQUATE(11)
LTRED       EQUATE(12)
LTMAGENTA   EQUATE(13)
YELLOW      EQUATE(14)
WHITE       EQUATE(15)

NULLWIN     EQUATE(0)

ASCITRANS   EQUATE(0)
XMODEM      EQUATE(1)
XMODEM1K    EQUATE(2)
XMODEM1KG   EQUATE(3)
YMODEM      EQUATE(4)
YMODEMG     EQUATE(5)
ZMODEM      EQUATE(6)

! Supported Terminal Types

ANSI        EQUATE(0)
TTY         EQUATE(1)
VT100       EQUATE(2)
IBM3161     EQUATE(3)

NUMTERMTYP  EQUATE(4)

TTGROUP     GROUP,PRE(TTGRP)
               STRING('IBM ANSI')
               STRING('TTY     ')
               STRING('VT100   ')
               STRING('IBM3161 ')
            End

TermTypeString  STRING(8),DIM(4),OVER(TTGROUP)


   ! structure for returning Port Parameters

COMPARAM    GROUP,TYPE,PRE(ComParam)
baud           LONG                       ! current baudrate
baudindex      USHORT                     ! index # in baudrate table
databits       USHORT                     ! current data bits
stop           USHORT                     ! current stop bits
parity         USHORT                     ! current parity
            End


   ! Queue Prototype for WaitForQueString

WaitQueStr  QUEUE,TYPE
WaitStr        CSTRING(100)               ! A String to Wait For
            End


AnswerThrd          LONG(0)               ! Thread # for Phone Monitor
AThrdParentWin     &WINDOW                ! Pointer to Parent Window Struct

AnswerThrdGrp   GROUP,Pre(ATHRD)
ParentHandle        UNSIGNED              ! Parent Window Handle
ParentThread        LONG                  ! Parent Thread (Normally 1)
PortNum             SHORT                 ! Com Port to use (0 Based)
NumRings            SHORT                 ! # of Rings to Wait For
ConnectWait         SHORT                 ! Seconds to Wait for Carrier after Answering
ModemBaud           LONG                  ! Caller Connect Rate
LockBaud            SHORT                 ! True to Lock the Baud Rate
StartMinimized      SHORT                 ! True to start Minimized
ModemStr            CSTRING(256)          ! Modem Connect String
                End

EVENT:StopMonitoring  EQUATE(507h)        ! Sent to Answer routine to stop monitoring
EVENT:Connected       EQUATE(508h)        ! Sent by Answer routine to tell us we are Connected
EVENT:PhoneMonClosed  EQUATE(509h)        ! Sent by Answer routine if it was closed

EVENT:GenConfig     EQUATE(600h)          ! Tell App Window to start Gen Config Form
EVENT:PortConfig    EQUATE(601h)          ! Tell App Window to start Port Config Form
EVENT:StartSplash   EQUATE(602h)          ! Tell App Window to start Splash Screen

EVENT:WM_CHAR       EQUATE(610h)          ! Received a keyboard character for Terminal
EVENT:GOT_CAR       EQUATE(611h)          ! Sent if Carrier Detect Changed (have carrier)
EVENT:NO_CAR        EQUATE(612h)          ! Sent if Carrier Detect Changed (don't have carrier)
EVENT:DOORWAY_ON    EQUATE(613h)          ! Sent if going into Doorway Mode
EVENT:DOORWAY_OFF   EQUATE(614h)          ! Sent if going out of DoorWay Mode
EVENT:AUTO_ZMODEM   EQUATE(615h)          ! Sent if Auto Zmodem detected
EVENT:WM_COMM_CHAR  EQUATE(616h)          ! Tell Terminal we received a character
EVENT:FileUpdated   EQUATE(617h)          !

TermFrameWin        UNSIGNED(0)           ! User Defined Terminal Emulator
TermClientWin       UNSIGNED(0)           ! Window Handles
OldTermProc         LONG(0)               ! Address of original Terminal Proc
OldTermClientProc   LONG(0)               ! Address of original Terminal Client Proc
OldHookHandle       LONG(0)               ! Handle of our Message Loop Filter
DoorWayMode         SHORT(0)              ! True if in DoorWay Mode
CapturingTerm       SHORT(0)              ! True if Capturing Terminal Session
EchoChars           SHORT(0)              ! True if Echoing Characters
CLATermPort         SHORT(0)              ! Port # being used by built in Terminal Emulator

   ! Dialog Codes

WM_ERASEBKGND       EQUATE(0014h)         ! Sent when Background needs to be Repainted
WM_GETDLGCODE       EQUATE(0087h)         ! Find out what Dialog wants to do
WM_CHAR             EQUATE(0102h)         ! Keyboard Input message (blocked by Clarion)
WM_COMMNOTIFY       EQUATE(0044h)         ! Received character Event

DLGC_WANTARROWS     EQUATE(0001h)         ! Control wants arrow keys
DLGC_WANTTAB        EQUATE(0002h)         ! Control wants tab keys
DLGC_WANTALLKEYS    EQUATE(0004h)         ! Control wants all keys
DLGC_WANTMESSAGE    EQUATE(0004h)         ! Pass message to control
DLGC_HASSETSEL      EQUATE(0008h)         ! Understands EM_SETSEL message
DLGC_DEFPUSHBUTTON  EQUATE(0010h)         ! Default pushbutton
DLGC_UNDEFPUSHBUT   EQUATE(0020h)         ! Non-default pushbutton
DLGC_RADIOBUTTON    EQUATE(0040h)         ! Radio button
DLGC_WANTCHARS      EQUATE(0080h)         ! Want WM_CHAR messages
DLGC_STATIC         EQUATE(0100h)         ! Static item: don't include
DLGC_BUTTON         EQUATE(2000h)         ! Button item: can be checked

   ! Thread specific Variables and Equates

   ! Win 32 Equates

INFINITE                        EQUATE(-1)
INVALID_HANDLE_VALUE            EQUATE(-1)

   ! Win 32 Data Types

BOOL                            EQUATE(SIGNED)
DWORD                           EQUATE(ULONG)
HANDLE                          EQUATE(UNSIGNED)
LPTSTR                          EQUATE(CSTRING)

   ! Thread Priorities

THREAD_PRIORITY_IDLE            EQUATE(-15)
THREAD_PRIORITY_LOWEST          EQUATE(-2)
THREAD_PRIORITY_BELOW_NORMAL    EQUATE(-1)
THREAD_PRIORITY_NORMAL          EQUATE(0)
THREAD_PRIORITY_ABOVE_NORMAL    EQUATE(1)
THREAD_PRIORITY_HIGHEST         EQUATE(2)
THREAD_PRIORITY_TIME_CRITICAL   EQUATE(15)

   ! Directory Changes we can monitor

FILE_NOTIFY_CHANGE_FILE_NAME    EQUATE(1)
FILE_NOTIFY_CHANGE_DIR_NAME     EQUATE(2)
FILE_NOTIFY_CHANGE_ATTRIBUTES   EQUATE(4)
FILE_NOTIFY_CHANGE_SIZE         EQUATE(8)
FILE_NOTIFY_CHANGE_LAST_WRITE   EQUATE(010h)

   ! Variables for a Single Thread

thdAppClosing                   SIGNED(0)        ! Set to 1 to tell Thread to Exit
thdThreadHandle                 UNSIGNED(0)      ! Thread Handle from Create Thread
thdThreadID                     ULONG(0)         ! Unique Thread ID
thdEventHandle                  UNSIGNED(0)      ! Handle of Event to Monitor
