
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_COUNT	4

int main (int argc, char *argv[])
{
	int rc;
	struct linearbuffers_encoder *encoder;
	const struct linearbuffers_output *output;

	size_t i;

	int8_t int8s[ARRAY_COUNT];
	int16_t int16s[ARRAY_COUNT];
	int32_t int32s[ARRAY_COUNT];
	int64_t int64s[ARRAY_COUNT];

	uint8_t uint8s[ARRAY_COUNT];
	uint16_t uint16s[ARRAY_COUNT];
	uint32_t uint32s[ARRAY_COUNT];
	uint64_t uint64s[ARRAY_COUNT];

	linearbuffers_a_enum_t enums[ARRAY_COUNT];

	char *strings[ARRAY_COUNT];

	uint64_t linearized_length;
	const char *linearized_buffer;

	(void) argc;
	(void) argv;

	srand(time(NULL));

	for (i = 0; i < sizeof(int8s) / sizeof(int8s[0]); i++) {
		int8s[i] = rand();
	}
	for (i = 0; i < sizeof(int16s) / sizeof(int16s[0]); i++) {
		int16s[i] = rand();
	}
	for (i = 0; i < sizeof(int32s) / sizeof(int32s[0]); i++) {
		int32s[i] = rand();
	}
	for (i = 0; i < sizeof(int64s) / sizeof(int64s[0]); i++) {
		int64s[i] = ((int64_t) rand() << 32) | rand();
	}

	for (i = 0; i < sizeof(uint8s) / sizeof(uint8s[0]); i++) {
		uint8s[i] = rand();
	}
	for (i = 0; i < sizeof(uint16s) / sizeof(uint16s[0]); i++) {
		uint16s[i] = rand();
	}
	for (i = 0; i < sizeof(uint32s) / sizeof(uint32s[0]); i++) {
		uint32s[i] = rand();
	}
	for (i = 0; i < sizeof(uint64s) / sizeof(uint64s[0]); i++) {
		uint64s[i] = ((uint64_t) rand() << 32) | rand();
	}

	for (i = 0; i < sizeof(strings) / sizeof(strings[0]); i++) {
		asprintf(&strings[i], "string-%zd", i);
	}

	for (i = 0; i < sizeof(enums) / sizeof(enums[0]); i++) {
		enums[i] = i;
	}

	encoder = linearbuffers_encoder_create(NULL);
	if (encoder == NULL) {
		fprintf(stderr, "can not create linearbuffers encoder\n");
		goto bail;
	}

	rc  = linearbuffers_output_start(encoder);
	rc |= linearbuffers_output_int8s_create(encoder, int8s, sizeof(int8s) / sizeof(int8s[0]));
	rc |= linearbuffers_output_int16s_create(encoder, int16s, sizeof(int16s) / sizeof(int16s[0]));
	rc |= linearbuffers_output_int32s_create(encoder, int32s, sizeof(int32s) / sizeof(int32s[0]));
	rc |= linearbuffers_output_int64s_create(encoder, int64s, sizeof(int64s) / sizeof(int64s[0]));
	rc |= linearbuffers_output_uint8s_create(encoder, uint8s, sizeof(uint8s) / sizeof(uint8s[0]));
	rc |= linearbuffers_output_uint16s_create(encoder, uint16s, sizeof(uint16s) / sizeof(uint16s[0]));
	rc |= linearbuffers_output_uint32s_create(encoder, uint32s, sizeof(uint32s) / sizeof(uint32s[0]));
	rc |= linearbuffers_output_uint64s_create(encoder, uint64s, sizeof(uint64s) / sizeof(uint64s[0]));
	rc |= linearbuffers_output_strings_create(encoder, (const char **) strings, sizeof(strings) / sizeof(strings[0]));
	rc |= linearbuffers_output_enums_create(encoder, enums, sizeof(enums) / sizeof(enums[0]));
	rc |= linearbuffers_a_table_vector_start(encoder);
	for (i = 0; i < ARRAY_COUNT; i++) {
		rc |= linearbuffers_a_table_start(encoder);
		rc |= linearbuffers_a_table_int8_set(encoder, int8s[i]);
		rc |= linearbuffers_a_table_int16_set(encoder, int16s[i]);
		rc |= linearbuffers_a_table_int32_set(encoder, int32s[i]);
		rc |= linearbuffers_a_table_int64_set(encoder, int64s[i]);
		rc |= linearbuffers_a_table_uint8_set(encoder, uint8s[i]);
		rc |= linearbuffers_a_table_uint16_set(encoder, uint16s[i]);
		rc |= linearbuffers_a_table_uint32_set(encoder, uint32s[i]);
		rc |= linearbuffers_a_table_uint64_set(encoder, uint64s[i]);
		rc |= linearbuffers_a_table_string_create(encoder, strings[i]);
		rc |= linearbuffers_a_table_anum_set(encoder, enums[i]);
		rc |= linearbuffers_a_table_vector_push(encoder, linearbuffers_a_table_end(encoder));
	}
	rc |= linearbuffers_output_tables_set(encoder, linearbuffers_a_table_vector_end(encoder));
	rc |= linearbuffers_output_finish(encoder);
	if (rc != 0) {
		fprintf(stderr, "can not encode output\n");
		goto bail;
	}

	linearized_buffer = linearbuffers_encoder_linearized(encoder, &linearized_length);
	if (linearized_buffer == NULL) {
		fprintf(stderr, "can not get linearized buffer\n");
		goto bail;
	}
	fprintf(stderr, "linearized: %p, length: %" PRIu64 "\n", linearized_buffer, linearized_length);

        output = linearbuffers_output_decode(linearized_buffer, linearized_length);
        if (output == NULL) {
                fprintf(stderr, "decoder failed: linearbuffers_output_decode\n");
                goto bail;
        }
        linearbuffers_output_jsonify(output, LINEARBUFFERS_JSONIFY_FLAG_DEFAULT, (int (*) (void *context, const char *fmt, ...)) fprintf, stderr);

	output = linearbuffers_output_decode(linearized_buffer, linearized_length);
	if (output == NULL) {
		fprintf(stderr, "decoder failed: linearbuffers_output_decode\n");
		goto bail;
	}

	if (linearbuffers_output_int8s_get_count(output) != sizeof(int8s) / sizeof(int8s[0])) {
		fprintf(stderr, "decoder failed: linearbuffers_output_int8s_get_count\n");
		goto bail;
	}
	if (linearbuffers_output_int8s_get_length(output) != sizeof(int8s)) {
		fprintf(stderr, "decoder failed: linearbuffers_output_int8s_get_length\n");
		goto bail;
	}
	if (memcmp(linearbuffers_output_int8s_get_values(output), int8s, sizeof(int8s))) {
		fprintf(stderr, "decoder failed: linearbuffers_output_int8s_get_values\n");
		goto bail;
	}
	for (i = 0; i < linearbuffers_output_int8s_get_count(output); i++) {
		if (int8s[i] != linearbuffers_output_int8s_get_at(output, i)) {
			fprintf(stderr, "decoder failed: linearbuffers_output_int8s_get_at\n");
			goto bail;
		}
	}

	if (linearbuffers_output_int16s_get_count(output) != sizeof(int16s) / sizeof(int16s[0])) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	if (linearbuffers_output_int16s_get_length(output) != sizeof(int16s)) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	if (memcmp(linearbuffers_output_int16s_get_values(output), int16s, sizeof(int16s))) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	for (i = 0; i < linearbuffers_output_int16s_get_count(output); i++) {
		if (int16s[i] != linearbuffers_output_int16s_get_at(output, i)) {
			fprintf(stderr, "decoder failed\n");
			goto bail;
		}
	}

	if (linearbuffers_output_int32s_get_count(output) != sizeof(int32s) / sizeof(int32s[0])) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	if (linearbuffers_output_int32s_get_length(output) != sizeof(int32s)) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	if (memcmp(linearbuffers_output_int32s_get_values(output), int32s, sizeof(int32s))) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	for (i = 0; i < linearbuffers_output_int32s_get_count(output); i++) {
		if (int32s[i] != linearbuffers_output_int32s_get_at(output, i)) {
			fprintf(stderr, "decoder failed\n");
			goto bail;
		}
	}

	if (linearbuffers_output_int64s_get_count(output) != sizeof(int64s) / sizeof(int64s[0])) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	if (linearbuffers_output_int64s_get_length(output) != sizeof(int64s)) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	if (memcmp(linearbuffers_output_int64s_get_values(output), int64s, sizeof(int64s))) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	for (i = 0; i < linearbuffers_output_int64s_get_count(output); i++) {
		if (int64s[i] != linearbuffers_output_int64s_get_at(output, i)) {
			fprintf(stderr, "decoder failed\n");
			goto bail;
		}
	}

	if (linearbuffers_output_uint8s_get_count(output) != sizeof(uint8s) / sizeof(uint8s[0])) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	if (linearbuffers_output_uint8s_get_length(output) != sizeof(uint8s)) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	if (memcmp(linearbuffers_output_uint8s_get_values(output), uint8s, sizeof(uint8s))) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	for (i = 0; i < linearbuffers_output_uint8s_get_count(output); i++) {
		if (uint8s[i] != linearbuffers_output_uint8s_get_at(output, i)) {
			fprintf(stderr, "decoder failed\n");
			goto bail;
		}
	}

	if (linearbuffers_output_uint16s_get_count(output) != sizeof(uint16s) / sizeof(uint16s[0])) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	if (linearbuffers_output_uint16s_get_length(output) != sizeof(uint16s)) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	if (memcmp(linearbuffers_output_uint16s_get_values(output), uint16s, sizeof(uint16s))) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	for (i = 0; i < linearbuffers_output_uint16s_get_count(output); i++) {
		if (uint16s[i] != linearbuffers_output_uint16s_get_at(output, i)) {
			fprintf(stderr, "decoder failed\n");
			goto bail;
		}
	}

	if (linearbuffers_output_uint32s_get_count(output) != sizeof(uint32s) / sizeof(uint32s[0])) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	if (linearbuffers_output_uint32s_get_length(output) != sizeof(uint32s)) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	if (memcmp(linearbuffers_output_uint32s_get_values(output), uint32s, sizeof(uint32s))) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	for (i = 0; i < linearbuffers_output_uint32s_get_count(output); i++) {
		if (uint32s[i] != linearbuffers_output_uint32s_get_at(output, i)) {
			fprintf(stderr, "decoder failed\n");
			goto bail;
		}
	}

	if (linearbuffers_output_uint64s_get_count(output) != sizeof(uint64s) / sizeof(uint64s[0])) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	if (linearbuffers_output_uint64s_get_length(output) != sizeof(uint64s)) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	if (memcmp(linearbuffers_output_uint64s_get_values(output), uint64s, sizeof(uint64s))) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	for (i = 0; i < linearbuffers_output_uint64s_get_count(output); i++) {
		if (uint64s[i] != linearbuffers_output_uint64s_get_at(output, i)) {
			fprintf(stderr, "decoder failed\n");
			goto bail;
		}
	}

	if (linearbuffers_output_strings_get_count(output) != sizeof(strings) / sizeof(strings[0])) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	for (i = 0; i < linearbuffers_output_strings_get_count(output); i++) {
		if (strcmp(strings[i], linearbuffers_output_strings_get_at(output, i)) != 0) {
			fprintf(stderr, "decoder failed\n");
			goto bail;
		}
	}

	if (linearbuffers_output_enums_get_count(output) != sizeof(enums) / sizeof(enums[0])) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	if (linearbuffers_output_enums_get_length(output) != sizeof(enums)) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	if (memcmp(linearbuffers_output_enums_get_values(output), enums, sizeof(enums))) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	for (i = 0; i < linearbuffers_output_enums_get_count(output); i++) {
		if (enums[i] != linearbuffers_output_enums_get_at(output, i)) {
			fprintf(stderr, "decoder failed\n");
			goto bail;
		}
	}

	if (linearbuffers_output_tables_get_count(output) != ARRAY_COUNT) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	for (i = 0; i < linearbuffers_output_tables_get_count(output); i++) {
		const struct linearbuffers_a_table *a_table = linearbuffers_output_tables_get_at(output, i);
		if (linearbuffers_a_table_int8_get(a_table) != int8s[i]) {
			fprintf(stderr, "decoder failed\n");
			goto bail;
		}
		if (linearbuffers_a_table_int16_get(a_table) != int16s[i]) {
			fprintf(stderr, "decoder failed\n");
			goto bail;
		}
		if (linearbuffers_a_table_int32_get(a_table) != int32s[i]) {
			fprintf(stderr, "decoder failed\n");
			goto bail;
		}
		if (linearbuffers_a_table_int64_get(a_table) != int64s[i]) {
			fprintf(stderr, "decoder failed\n");
			goto bail;
		}
		if (linearbuffers_a_table_uint8_get(a_table) != uint8s[i]) {
			fprintf(stderr, "decoder failed\n");
			goto bail;
		}
		if (linearbuffers_a_table_uint16_get(a_table) != uint16s[i]) {
			fprintf(stderr, "decoder failed\n");
			goto bail;
		}
		if (linearbuffers_a_table_uint32_get(a_table) != uint32s[i]) {
			fprintf(stderr, "decoder failed\n");
			goto bail;
		}
		if (linearbuffers_a_table_uint64_get(a_table) != uint64s[i]) {
			fprintf(stderr, "decoder failed\n");
			goto bail;
		}
		if (strcmp(linearbuffers_a_table_string_get_value(a_table), strings[i]) != 0) {
			fprintf(stderr, "decoder failed\n");
			goto bail;
		}
		if (linearbuffers_a_table_anum_get(a_table) != enums[i]) {
			fprintf(stderr, "decoder failed\n");
			goto bail;
		}
	}

	linearbuffers_encoder_destroy(encoder);

	for (i = 0; i < sizeof(strings) / sizeof(strings[0]); i++) {
		free(strings[i]);
	}
	return 0;
bail:	if (encoder != NULL) {
		linearbuffers_encoder_destroy(encoder);
	}
	return -1;
}
