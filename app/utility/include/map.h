#ifndef _UNIRAIL_MAP_H
	#define _UNIRAIL_MAP_H

	/**
	 * @struct position_t
	 * @brief Represents a position on the rail
	 * @var bal The balise ID
	 * @var pos_r The position relative to the balise on the rail
	 */
	typedef struct {
		int bal;
		float pos_r;
	} position_t;

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