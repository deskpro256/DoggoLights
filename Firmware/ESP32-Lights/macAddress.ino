String getMacLastHalf() {
    uint64_t macAddress = ESP.getEfuseMac();  // Get the MAC address (64-bit value)
    char macStr[13];  // Buffer to hold the formatted MAC string

    // Extract the second half (last 3 bytes) of the MAC address
    uint32_t macLastHalf = (uint32_t)(macAddress >> 24);

    // Format the last half of the MAC into a string
    snprintf(macStr, sizeof(macStr), "%06X", macLastHalf);

    // Return as a String object
    return String(macStr);
}
