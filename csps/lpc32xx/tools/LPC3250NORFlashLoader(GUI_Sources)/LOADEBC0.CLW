  MEMBER('loader.clw')

  INCLUDE('ABFILE.INC'),ONCE

  MAP
LOADEBC0:DctInit    PROCEDURE
LOADEBC0:DctKill    PROCEDURE
LOADEBC0:FilesInit  PROCEDURE
  END

Hide:Access:bin      CLASS(FileManager)
Init                   PROCEDURE
Kill                   PROCEDURE
                     END

Hide:Relate:bin      CLASS(RelationManager)
Init                   PROCEDURE
Kill                   PROCEDURE,VIRTUAL
                     END

Hide:Access:loader   CLASS(FileManager)
Init                   PROCEDURE
Kill                   PROCEDURE
                     END

Hide:Relate:loader   CLASS(RelationManager)
Init                   PROCEDURE
Kill                   PROCEDURE,VIRTUAL
                     END

LOADEBC0:DctInit PROCEDURE
  CODE
  Relate:bin &= Hide:Relate:bin
  Relate:loader &= Hide:Relate:loader

LOADEBC0:FilesInit PROCEDURE
  CODE
  Hide:Relate:bin.Init
  Hide:Relate:loader.Init


LOADEBC0:DctKill PROCEDURE
  CODE
  Hide:Relate:bin.Kill
  Hide:Relate:loader.Kill


Hide:Access:bin.Init PROCEDURE
  CODE
  PARENT.Init(bin,GlobalErrors)
  SELF.FileName &= filename
  SELF.Buffer &= BIN:Record
  SELF.Create = 1
  SELF.LockRecover = 10
  Access:bin &= SELF

Hide:Access:bin.Kill PROCEDURE
  CODE
  PARENT.Kill
  Access:bin &= NULL

Hide:Access:loader.Init PROCEDURE
  CODE
  PARENT.Init(loader,GlobalErrors)
  SELF.Buffer &= LOA:Record
  SELF.Create = 0
  SELF.LockRecover = 10
  Access:loader &= SELF

Hide:Access:loader.Kill PROCEDURE
  CODE
  PARENT.Kill
  Access:loader &= NULL

Hide:Relate:bin.Init PROCEDURE
  CODE
  Hide:Access:bin.Init
  PARENT.Init(Access:bin,1)

Hide:Relate:bin.Kill PROCEDURE
  CODE
  Hide:Access:bin.Kill
  PARENT.Kill
  Relate:bin &= NULL

Hide:Relate:loader.Init PROCEDURE
  CODE
  Hide:Access:loader.Init
  PARENT.Init(Access:loader,1)

Hide:Relate:loader.Kill PROCEDURE
  CODE
  Hide:Access:loader.Kill
  PARENT.Kill
  Relate:loader &= NULL
