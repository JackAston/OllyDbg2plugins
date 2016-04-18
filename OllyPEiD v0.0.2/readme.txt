Copy OllyPEiD.dll into the Plugins directory. Note: the userdb.txt must be in
the same directory as the plugin unless otherwise specified in ollydbg.ini.

Keep in mind this is an alpha release. I have not fully tested and taken the
time to remove all possibility for bugs so there may be a few lingering around.

/*******************************************************************************
 * Things to change as I think of them...
 * [ ] = To do
 * [?] = Might be a good idea?
 * [!] = Implemented
 * [+] = Added
 * [-] = Removed
 * [*] = Changed
 * [~] = Almost there...
 *
 *
 *
 * Version 0.0.2-alpha
 * [+] Added ScanFile routine to search database
 *
 * Version 0.0.1-alpha
 * [+] Base code
 *
 *
 * -----------------------------------------------------------------------------
 * TODO
 * -----------------------------------------------------------------------------
 * [!] Refactor ScanFile code - v0.0.2
 * [~] Better analysis on signature database in case formatting is a little off
 * [?] Add OllyPEiD Log window
 * [ ] Improve error checking for most everything
 * [ ] Add Options window
 * [ ] Assumes ep_only = true. Automatically determine from Options menu
 * [?] Option: Scan On Analyse
 * [ ] Option: Scan On Module Load
 * [ ] Option: specify max_sig_len
 *
 ******************************************************************************/