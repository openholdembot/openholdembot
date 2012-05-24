
@Echo This batch-file will try to run the OpenPPL-testsuite,
@Echo using "release\OpenPPL.Exe" and the "ok_*.txt"-files
@Echo in "OpenPPL/TestSuite".
@Echo It won't try to translate tehe "error_*.txt"-files,
@Echo so you shouldn't get any error-messages.

Pause

@IF NOT EXIST Release\OpenPPL.Exe @ECHO ERROR: Release\OpenPPL.Exe does not exist
@IF NOT EXIST Release\OpenPPL.Exe @EXIT 

@COPY OpenPPL_Library\OpenPPL_Library.ohf .

FOR %%f IN (OpenPPL\TestSuite\ok*.txt) DO Release\OpenPPL.Exe %%f > Temp.OHF

@DEL OpenPPL_Library.ohf
@DEL Temp.OHF