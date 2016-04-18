
== Files
- OllyDumpEx_Od11.dll
  for OllyDbg version 1.10 (1.10 tested)
- OllyDumpEx_Od20.dll
  for OllyDbg version 2.01 (2.01a4 tested, EXPERIMENTAL)
- OllyDumpEx_Imm17.dll
  for Immunity Debugger version 1.7x or lower (1.73 tested)
- OllyDumpEx_Imm18.dll
  for Immunity Debugger version 1.8x or higher (1.83 tested)


== Known Issue
- Image type attribute is always readonly 
  OllyDbg 1.10 and Immunity Debugger not set "t_memory.access" correctly.
- OllyDbg version 2.01 Support
  PDK with documentation not released yet.
  Just guess from sample bookmark plugin and ollydbg.exe file.
  This version support is EXPERIMENTAL. I'll check and rebuild after PDK released.
- May not work almost feature under ASLR enabled environment
  Fix later.
  

== Changelog
- v0.90 / 2011-08-24
  Add: Support OllyDbg version 2 plugin interface (EXPERIMENTAL)
  Improve: Rewrite Wide/Multibyte-Character support code
  Improve: Decode CopyOnWrite page attribute
  Bugfix: Detect working directory (Wide-Character only)
- v0.80 / 2011-08-01
  Add: Support Immunity Debugger version 1.8x or higher
  Improve: Data Directory rebuild option (check rewrite range)
  Improve: Always round up PE header size to 0x1000 (ImportRec not extend itself)
  Bugfix: TLS Data Directory ignored
- v0.70 / 2011-07-07
  Add: Support Immunity Debugger version 1.7x or lower
  Improve: Data Directory rebuild option (support ImportTable)
  Improve: Image Base Address alignment checking
  Improve: Virtual Offset Address alignment checking
- v0.60
  Improve: DeSelect is too slow when Auto Adjust Image Base option enabled
  Add: Virtual Offset Address overlap checking
- v0.50
  Improve: Data Directory rebuild option (support ResourceDirectory)
  Add: Auto Adjust Image Base Address option
- v0.40
  Add: Data Directory rebuild option for RVA Adjust (support ExportTable)
  Improve: Invalid Image Base checking
  Improve: Virtual Offset overlap checking
- v0.30
  Add: Section sort by Virtual Offset
  Add: Fill Virtual Offset Hole option
- v0.20
  Bugfix: Fix many bugs 
- v0.10
  Initial version


== Bug Report
  Please try latest version before report. 
  With your environment detail, logs and way to reproduce.
    http://low-priority.appspot.com/ollydumpex/
    lowprio20/_at_/gmail/_dot_/com

