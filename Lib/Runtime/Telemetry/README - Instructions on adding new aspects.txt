1. Ensure you have `release` and `test` (debug) build subfolders with the `sources` file with the correct paths specified.
2. Ensure you've added the new directory's name into the `dirs` file.
3. Ensure you've set a new name for the .lib file in the `sources.inc` file in your new directory.
4. Ensure this library name is referenced in `$\inetcore\jscript\jslib.inc` file.
5. Ensure the subfolder has a `dirs` folder too, referencing `release` and `test`.
6. Ensure that the `$\inetcore\jscript\runtime\dirs` file references the `Telemetry` folder still.
7. Ensure that all of these `dirs`, `sources`, and `sources.inc` files are checked-out in SD, otherwise they might be overwritten during a sync and won't be checked-in.