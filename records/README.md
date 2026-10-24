# Records

Each verified change on this branch has a record: what was checked, the result, and how to repeat
the check yourself. Open items are listed, not hidden.

- `reply-crc.md`: check each Bluetooth LE reply's CRC before it is used (commit `d26421e`)

The other changes on this branch: `5b37b72` adds the host test stand-ins (`host/`), `5cf8df4` adds
the first native suites (14 cases), and `5e44c0c` fixes a command on an unmatched MQTT topic queueing
a Bluetooth write; its test, `test/test_unmatched_topic`, fails on `5cf8df4` and passes from
`5e44c0c` on.
