! Status Messages that CLACom can set during a File Transfer.

! These are placed inside of a Group structure so that they
!    can be initialized during Compile time instead of
!    Run time.

statStatus_Grp  GROUP
        STRING('                                ')  !  1
        STRING('Transfer Canceled               ')  !  2
        STRING('Receiver Canceled               ')  !  3
        STRING('Time Out                        ')  !  4
        STRING('Unknown Error                   ')  !  5
        STRING('CRC Error                       ')  !  6
        STRING('Sender Canceled                 ')  !  7
        STRING('Long Packet                     ')  !  8
        STRING('Too Many Errors                 ')  !  9
        STRING('Bad File Position               ')  ! 10
        STRING('Could Not Open File             ')  ! 11
        STRING('Waiting For Receiver            ')  ! 12
        STRING('Sending ZRQINIT                 ')  ! 13
        STRING('Sending Filename                ')  ! 14
        STRING('Sending File                    ')  ! 15
        STRING('Waiting For FileName            ')  ! 16
        STRING('Receiving File                  ')  ! 17
        STRING('Lost Carrier                    ')  ! 18
        STRING('Retransmitting Block            ')  ! 19
        STRING('Transfer Completed              ')  ! 20
        STRING('Bad Block                       ')  ! 21
        STRING('Checksum Error                  ')  ! 22
        STRING('Error Writing To File           ')  ! 23
        STRING('No Response to NAK              ')  ! 24
     End

! All strings must be the same length. If you increase
!   the length of any single string, you must also
!   increase the length of the other strings. Do not
!   put anything inside of String 1 (other than spaces),
!   as it is used to clear the Status field.

! New updates of CLACom will not overwrite this file,
!   so any changes you make will be left as is.

! If you need to change the length of the Strings
!   due to translation issues, set the statStatusLen
!   EQUATE to the size of the Strings. For instance,
!   if you need to expand the strings by 2 bytes,
!   change the EQUATE from 32 to 34.

statStatusLen EQUATE(32)                         ! Length of each string
statStatusNum EQUATE(24)                         ! Number of strings

statStatus_Msg  STRING(statStatusLen),DIM(statStatusNum),OVER(statStatus_Grp)

! You may use these variables on your Status Window.
!   They are set by the CallBack function. Make sure
!   your Status Window initializes them (or that they
!   are initialized before you start a File Transfer).
!   Otherwise, they will show whatever value they had
!   the last time they were used.

statFileName   STRING(256)                       ! Name of File
statSize       ULONG                             ! Size of File
statBytes      ULONG                             ! Bytes Sent/Received
statCPS        SIGNED                            ! Characters per second
statErrors     SIGNED                            ! Number of Errors
statStatus     STRING(statStatusLen)             ! Status Message (one of the Strings above)
statProgress   ULONG                             ! Progress Bar

! These are used internally by the CallBack function
! and the Status Window.

statAbort      SIGNED                            ! True if User cancels transfer
statButProc    LONG(0)                           ! Address of original Button Window Proc
statWinThread  LONG(0)                           ! Thread Number of Status Window
statWinRef     &WINDOW                           ! Status Window structure
statProgCntrl  LONG                              ! Progress Bar Control ID

