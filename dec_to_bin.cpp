char* dec_to_bin(uint32_t val) {
	char* bin_str = calloc(sizeof(char) * 33); //33 for null terminator
	for(int i = 31; i >= 0; --i) {
		sprintf((bin_str + i), "%1d", val % 2);
		val /= 2;
	}
	return bin_str;
	// return (val == 0) ? 0 : (val % 2) + (10 * dec_to_bin(val / 2));
}
