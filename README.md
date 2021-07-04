# res_file

Small commandline tool that can extract and pack Age of Empires resource files (.drs), using resource manifests (.rm) similar to how it originally worked.

## usage

extracting:
when no manifest is passed:
* `res_file x interfac.drs` -> extract to "interfac"
* `res_file x interfac.drs bla` -> extra to "bla"

with manifest
* `res_file x interfac.rm` -> extract "interfac.drs" to "interfac"
* `res_file x interfac.rm bla` -> extract "interfac.drs" to "bla"
* `res_file x interfac.rm bla bla_in` -> extract "bla_in\interfac.drs" to "bla"

making:
* `res_file m interfac.rm` -> create "interfac.drs" in current folder from manifest, with files in folder "interfac"
* `res_file m interfac.rm bla` -> create "interfac.drs" with files in folder "bla\interfac"
* `res_file m interfac.rm bla bla2` -> create "bla2\interfac.drs" with files in folder "bla\interfac"
