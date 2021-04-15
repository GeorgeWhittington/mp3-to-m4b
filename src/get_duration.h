#ifndef GET_DURATION
#define GET_DURATION

#include <string>

long long int get_audio_duration(std::string filename);
double get_audio_duration_exact(std::string filename);

#endif