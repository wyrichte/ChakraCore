To generate tests, follow following steps

1. Checkout all files in this directory
>sd edit *
   
2. Run Generate tests script
>cscript GenerateTests.js

3. Create the baseline files
>createBaseLine.bat

4. Revert any unchanged files
>sd revert -a

5. Review opened files and diff. Might want to add new files created (if any)
