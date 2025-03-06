#ifndef _UNIRAIL_GESTION_RESSOURCES_H
	#define _UNIRAIL_GESTION_RESSOURCES_H

	#define NB_RESSOURCES 6

	/**
	 * @brief Locks the resources specified by the mask. Blocks until the resources are available.
	 * @param res_mask The mask of the resources to lock
	 */
	void lock_ressources(unsigned char res_mask);

	/**
	 * @brief Unlocks the resources specified by the mask.
	 * @param res_mask The mask of the resources to unlock
	 */
	void unlock_ressources(unsigned char res_mask);

#endif