// SPDX-License-Identifier: GPL-2.0
#ifndef DIVE_H
#define DIVE_H

// dive and dive computer related structures and helpers

#include "divemode.h"
#include "divecomputer.h"
#include "equipment.h"
#include "picture.h" // TODO: remove

#include <string>
#include <vector>

extern int last_xml_version;

extern const char *divemode_text_ui[];
extern const char *divemode_text[];

struct dive_site;
struct dive_table;
struct dive_trip;
struct full_text_cache;
struct event;
struct trip_table;

struct dive {
	struct dive_trip *divetrip = nullptr;
	timestamp_t when = 0;
	struct dive_site *dive_site = nullptr;
	char *notes = nullptr;
	char *diveguide = nullptr, *buddy = nullptr;
	cylinder_table cylinders;
	weightsystem_table weightsystems;
	char *suit = nullptr;
	int number = 0;
	int rating = 0;
	int wavesize = 0, current = 0, visibility = 0, surge = 0, chill = 0; /* 0 - 5 star ratings */
	int sac = 0, otu = 0, cns = 0, maxcns = 0;

	/* Calculated based on dive computer data */
	temperature_t mintemp, maxtemp, watertemp, airtemp;
	depth_t maxdepth, meandepth;
	pressure_t surface_pressure;
	duration_t duration;
	int salinity = 0; // kg per 10000 l
	int user_salinity = 0; // water density reflecting a user-specified type

	struct tag_entry *tag_list = nullptr;
	std::vector<divecomputer> dcs; // Attn: pointers to divecomputers are not stable!
	int id = 0; // unique ID for this dive
	struct picture_table pictures = { };
	unsigned char git_id[20] = {};
	bool notrip = false; /* Don't autogroup this dive to a trip */
	bool selected = false;
	bool hidden_by_filter = false;
	struct full_text_cache *full_text = nullptr; /* word cache for full text search */
	bool invalid = false;

	dive();
	~dive();
	dive(dive &&);
	dive &operator=(const dive &);
};

/* For the top-level list: an entry is either a dive or a trip */
struct dive_or_trip {
	struct dive *dive;
	struct dive_trip *trip;
};

extern void invalidate_dive_cache(struct dive *dive);
extern bool dive_cache_is_valid(const struct dive *dive);

extern int get_cylinder_idx_by_use(const struct dive *dive, enum cylinderuse cylinder_use_type);
extern void cylinder_renumber(struct dive *dive, int mapping[]);
extern int same_gasmix_cylinder(const cylinder_t &cyl, int cylid, const struct dive *dive, bool check_unused);

/* when selectively copying dive information, which parts should be copied? */
struct dive_components {
	unsigned int divesite : 1;
	unsigned int notes : 1;
	unsigned int diveguide : 1;
	unsigned int buddy : 1;
	unsigned int suit : 1;
	unsigned int rating : 1;
	unsigned int visibility : 1;
	unsigned int wavesize : 1;
	unsigned int current : 1;
	unsigned int surge : 1;
	unsigned int chill : 1;
	unsigned int tags : 1;
	unsigned int cylinders : 1;
	unsigned int weights : 1;
	unsigned int number : 1;
	unsigned int when : 1;
};

extern bool has_gaschange_event(const struct dive *dive, const struct divecomputer *dc, int idx);
extern int explicit_first_cylinder(const struct dive *dive, const struct divecomputer *dc);

extern fraction_t best_o2(depth_t depth, const struct dive *dive, bool in_planner);
extern fraction_t best_he(depth_t depth, const struct dive *dive, bool o2narcotic, fraction_t fo2);

extern int get_surface_pressure_in_mbar(const struct dive *dive, bool non_null);
extern int depth_to_mbar(int depth, const struct dive *dive);
extern double depth_to_mbarf(int depth, const struct dive *dive);
extern double depth_to_bar(int depth, const struct dive *dive);
extern double depth_to_atm(int depth, const struct dive *dive);
extern int rel_mbar_to_depth(int mbar, const struct dive *dive);
extern int mbar_to_depth(int mbar, const struct dive *dive);
extern depth_t gas_mod(struct gasmix mix, pressure_t po2_limit, const struct dive *dive, int roundto);
extern depth_t gas_mnd(struct gasmix mix, depth_t end, const struct dive *dive, int roundto);

extern struct dive *get_dive(int nr);
extern struct dive *get_dive_from_table(int nr, const struct dive_table *dt);
extern struct dive_site *get_dive_site_for_dive(const struct dive *dive);
extern std::string get_dive_country(const struct dive *dive);
extern std::string get_dive_location(const struct dive *dive);
extern unsigned int number_of_computers(const struct dive *dive);
extern struct divecomputer *get_dive_dc(struct dive *dive, int nr);
extern const struct divecomputer *get_dive_dc(const struct dive *dive, int nr);
extern timestamp_t dive_endtime(const struct dive *dive);
extern temperature_t dc_airtemp(const struct dive *dive);
extern temperature_t dc_watertemp(const struct dive *dive);

extern void set_git_prefs(const char *prefs);

extern struct dive *make_first_dc(const struct dive *d, int dc_number);
extern struct dive *clone_delete_divecomputer(const struct dive *d, int dc_number);
void split_divecomputer(const struct dive *src, int num, struct dive **out1, struct dive **out2);

/*
 * Iterate over each dive, with the first parameter being the index
 * iterator variable, and the second one being the dive one.
 *
 * I don't think anybody really wants the index, and we could make
 * it local to the for-loop, but that would make us requires C99.
 */
#define for_each_dive(_i, _x) \
	for ((_i) = 0; ((_x) = get_dive(_i)) != NULL; (_i)++)

extern struct dive *get_dive_by_uniq_id(int id);
extern int get_idx_by_uniq_id(int id);
extern bool dive_site_has_gps_location(const struct dive_site *ds);
extern int dive_has_gps_location(const struct dive *dive);
extern location_t dive_get_gps_location(const struct dive *d);

extern bool time_during_dive_with_offset(const struct dive *dive, timestamp_t when, timestamp_t offset);

extern int save_dives(const char *filename);
extern int save_dives_logic(const char *filename, bool select_only, bool anonymize);
extern int save_dive(FILE *f, struct dive *dive, bool anonymize);
extern int export_dives_xslt(const char *filename, bool selected, const int units, const char *export_xslt, bool anonymize);

extern int save_dive_sites_logic(const char *filename, const struct dive_site *sites[], int nr_sites, bool anonymize);

struct membuffer;
extern void save_one_dive_to_mb(struct membuffer *b, struct dive *dive, bool anonymize);

extern void subsurface_console_init();
extern void subsurface_console_exit();
extern bool subsurface_user_is_root();

extern void record_dive_to_table(struct dive *dive, struct dive_table *table);
extern void clear_dive(struct dive *dive);
extern void copy_dive(const struct dive *s, struct dive *d);
extern void selective_copy_dive(const struct dive *s, struct dive *d, struct dive_components what, bool clear);
extern struct std::unique_ptr<dive> move_dive(struct dive *s);

extern int legacy_format_o2pressures(const struct dive *dive, const struct divecomputer *dc);

extern bool dive_less_than(const struct dive *a, const struct dive *b);
extern bool dive_or_trip_less_than(struct dive_or_trip a, struct dive_or_trip b);
extern struct dive *fixup_dive(struct dive *dive);
extern pressure_t calculate_surface_pressure(const struct dive *dive);
extern pressure_t un_fixup_surface_pressure(const struct dive *d);
extern int get_dive_salinity(const struct dive *dive);
extern int dive_getUniqID();
extern int split_dive(const struct dive *dive, struct dive **new1, struct dive **new2);
extern int split_dive_at_time(const struct dive *dive, duration_t time, struct dive **new1, struct dive **new2);
extern struct dive *merge_dives(const struct dive *a, const struct dive *b, int offset, bool prefer_downloaded, struct dive_trip **trip, struct dive_site **site);
extern struct dive *try_to_merge(struct dive *a, struct dive *b, bool prefer_downloaded);
extern void copy_events_until(const struct dive *sd, struct dive *dd, int dcNr, int time);
extern void copy_used_cylinders(const struct dive *s, struct dive *d, bool used_only);
extern bool is_cylinder_used(const struct dive *dive, int idx);
extern bool is_cylinder_prot(const struct dive *dive, int idx);
extern void add_gas_switch_event(struct dive *dive, struct divecomputer *dc, int time, int idx);
extern struct event create_gas_switch_event(struct dive *dive, struct divecomputer *dc, int seconds, int idx);
extern void per_cylinder_mean_depth(const struct dive *dive, struct divecomputer *dc, int *mean, int *duration);
extern int get_cylinder_index(const struct dive *dive, const struct event &ev);
extern struct gasmix get_gasmix_from_event(const struct dive *, const struct event &ev);
extern bool cylinder_with_sensor_sample(const struct dive *dive, int cylinder_id);

/* UI related protopypes */

extern void invalidate_dive_cache(struct dive *dc);

extern int total_weight(const struct dive *);

extern bool is_planned(const struct dive *dive);
extern bool is_logged(const struct dive *dive);

/* Get gasmix at a given time */
extern struct gasmix get_gasmix_at_time(const struct dive &dive, const struct divecomputer &dc, duration_t time);

extern void update_setpoint_events(const struct dive *dive, struct divecomputer *dc);

/* Make pointers to dive and dive_trip "Qt metatypes" so that they can be passed through
 * QVariants and through QML.
 */
#include <QObject>
Q_DECLARE_METATYPE(struct dive *);

extern std::string existing_filename;

#endif // DIVE_H
