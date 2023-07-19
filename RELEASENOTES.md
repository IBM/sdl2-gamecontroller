# v1.0.10
- Add controller battery events
- Fix build issue with brew on Linux

# v1.0.9
- Add support for apple M1

# v1.0.8
- Issue warning for mismatch params instead of throwing error. Resolve issue where a param is passed to `rumble` or another function but the param was undefined and an error is thrown. This was particularly an issue for the player number which is often not supported.

# v1.0.7
- Add support for ASUS ROG Chakram mouse. (Requires SDL2 2.0.22)
- Add support for setting the polling interval

# v1.0.6
- Add typescript support
