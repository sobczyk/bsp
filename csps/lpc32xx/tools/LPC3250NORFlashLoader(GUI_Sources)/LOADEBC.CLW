  MEMBER('loader.clw')

  MAP
    MODULE('LOADEBC0.CLW')
LOADEBC0:DctInit             PROCEDURE
LOADEBC0:DctKill             PROCEDURE
LOADEBC0:FilesInit           PROCEDURE
    END
  END

DctInit PROCEDURE
  CODE
  LOADEBC0:DctInit
  LOADEBC0:FilesInit


DctKill PROCEDURE
  CODE
  LOADEBC0:DctKill

