#ifndef _UNIRAIL_MAP_H
	#define _UNIRAIL_MAP_H

	#include "position.h"

	extern const int B1[];
	extern const int B2[];
	extern const float D[];
	extern const int chemin3[];
	extern const int chemin2[];
	extern const int chemin1[];

	extern const int * chemins[3];
	extern const int tailles_chemins[3];

	float get_distance(position_t pos_current, position_t pos_destination, const int chemin_id);

#endif