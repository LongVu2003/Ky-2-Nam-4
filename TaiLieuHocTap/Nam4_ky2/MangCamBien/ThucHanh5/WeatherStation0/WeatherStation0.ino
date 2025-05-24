// Libraries for DHT sensor
#include <DHT.h>

// Edge Impulse Inferencing Library for your project
// Make sure this matches the name of the folder and header file from Edge Impulse
#include <longvu2003-project-1_inferencing.h>

// Define sensor pin and type for DHT22
#define DHTPIN 12     // Pin D12 for DHT22 data
#define DHTTYPE DHT22 // Sensor type
DHT dht(DHTPIN, DHTTYPE);

// Variables for sensor readings
float hum;
float temp_c;
float humidex;

// Timing for sensor readings
// This should generally align with EI_CLASSIFIER_INTERVAL_MS from your model
// or be a multiple of it if you are collecting multiple samples for a window.
// For DHT22, we read every 2 seconds.
const unsigned long SENSOR_READ_INTERVAL_MS = 2000;
static unsigned long last_sensor_read_ms = 0;

// Buffer to store features for inference.
// Its size is EI_CLASSIFIER_RAW_SAMPLE_COUNT * EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME.
// EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME should be 3 for (humidity, temperature, humidex).
// EI_CLASSIFIER_RAW_SAMPLE_COUNT is the number of (hum, temp, humidex) sets in a window.
// If RAW_SAMPLE_COUNT is 1, DSP_INPUT_FRAME_SIZE will be 3.
static float features_buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
static size_t current_feature_ix = 0; // Index for the features_buffer

/**
 * @brief Callback function for printf output from Edge Impulse SDK.
 * This function is expected by the Edge Impulse library.
 */
void ei_printf(const char *format, ...) {
    static char print_buf[1024] = { 0 };
    va_list args;
    va_start(args, format);
    int r = vsnprintf(print_buf, sizeof(print_buf), format, args);
    va_end(args);
    if (r > 0) {
        Serial.write(print_buf);
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 5000); // Wait for serial connection (helpful for some boards)
    Serial.println("Edge Impulse DHT22 Inferencing Started");

    dht.begin(); // Initialize DHT sensor
    delay(1000); // Allow sensor to stabilize

    // Check if the raw samples per frame is 3 (humidity, temperature, humidex)
    if (EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME != 3) {
        ei_printf("ERROR: Your model's 'Raw samples per frame' (%d) is not 3!\n", EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME);
        ei_printf("This project expects 3 features: humidity, temperature, and humidex.\n");
        ei_printf("Please check your Edge Impulse project configuration under 'Create impulse'.\n");
        while (1) delay(100); // Halt
    }

    // Print model information
    ei_printf("Edge Impulse model settings:\n");
    ei_printf("  Interval: %.2f ms.\n", (float)EI_CLASSIFIER_INTERVAL_MS);
    ei_printf("  Frame size (total features in window): %d\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    ei_printf("  Sample count (number of [hum,temp,humdex] sets per window): %d\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT);
    ei_printf("  Samples per frame (axes): %d\n", EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME);
    ei_printf("  Label count: %d\n", EI_CLASSIFIER_LABEL_COUNT);

    // Initialize anomaly detection model if it's part of your impulse
    #if EI_CLASSIFIER_HAS_ANOMALY == 1
        ei_printf("Initializing anomaly detection...\n");
        EI_IMPULSE_ERROR anomaly_init_res = init_anomaly_detection_from_model(); // Pass &result if your SDK version requires it
        if (anomaly_init_res != EI_IMPULSE_OK) {
            ei_printf("ERROR: Failed to initialize anomaly detector (%d)\n", anomaly_init_res);
        }
    #endif
}

void loop() {
    if (millis() - last_sensor_read_ms >= SENSOR_READ_INTERVAL_MS) {
        last_sensor_read_ms = millis();

        // 1. Read sensor data
        hum = dht.readHumidity();
        temp_c = dht.readTemperature();

        if (isnan(hum) || isnan(temp_c)) {
            // Optional: print a warning if sensor read fails
            // ei_printf("WARN: Failed to read from DHT sensor, skipping this sample.\n");
            return; // Skip this cycle if reading failed
        }

        // Calculate humidex (same formula as before)
        float e_component = 0.06 * hum * pow(10, (0.03 * temp_c));
        humidex = temp_c + (0.5555 * (e_component - 10.0));

        // 2. Add data to the features buffer
        // The order must be exactly the same as you defined in Edge Impulse Studio
        // and used for training (e.g., humidity, then temperature, then humidex).
        features_buffer[current_feature_ix++] = hum;
        features_buffer[current_feature_ix++] = temp_c;
        features_buffer[current_feature_ix++] = humidex;

        // Debug: Print current sample collected
        // ei_printf("Sample collected: %.1f, %.1f, %.1f\n", hum, temp_c, humidex);

        // 3. Check if the features buffer is full
        if (current_feature_ix == EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
            ei_printf("\nFeature buffer is full. Running inference...\n");

            // Create a signal object from the features buffer
            signal_t signal;
            int err = numpy::signal_from_buffer(features_buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
            if (err != 0) {
                ei_printf("ERROR: Failed to create signal from buffer (%d)\n", err);
                current_feature_ix = 0; // Reset buffer
                return;
            }

            // Placeholder for results
            ei_impulse_result_t result = {0};

            // 4. Run the classifier
            // Set the 'debug' flag to true for more detailed output from the classifier
            EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
            if (res != EI_IMPULSE_OK) {
                ei_printf("ERROR: Failed to run classifier (%d)\n", res);
                current_feature_ix = 0; // Reset buffer
                return;
            }

            // 5. Print inference results
            ei_printf("Predictions (DSP: %d ms., Classification: %d ms.",
                      result.timing.dsp, result.timing.classification);
            #if EI_CLASSIFIER_HAS_ANOMALY == 1
                ei_printf(", Anomaly: %d ms.", result.timing.anomaly);
            #endif
            ei_printf("):\n");

            for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
                ei_printf("  %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
            }

            #if EI_CLASSIFIER_HAS_ANOMALY == 1
                ei_printf("  Anomaly score: %.3f\n", result.anomaly);
            #endif

            // Reset features buffer index for the next inference cycle
            current_feature_ix = 0;
        }
        // Optional: Print progress if buffer is not yet full
        // else {
        //    int samples_collected = current_feature_ix / EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME;
        //    ei_printf("Collected %d/%d samples for current window.\n", samples_collected, EI_CLASSIFIER_RAW_SAMPLE_COUNT);
        // }
    }
}