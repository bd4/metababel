#include "component.h"
#include "dispatch.h"
#include <stdio.h>
#include <assert.h>

struct data_s {
  uint64_t event_1_calls_count;
  uint64_t event_2_calls_count;
};

typedef struct data_s data_t;

void btx_initialize_usr_data(common_data_t *common_data, void **usr_data)
{
  data_t *data = malloc(sizeof(data_t *));
  *usr_data = data;
  
  data->event_1_calls_count = 0;
  data->event_2_calls_count = 0;
}

void btx_finalize_usr_data(common_data_t *common_data, void *usr_data)
{
  data_t *data = (data_t *) usr_data;

  assert(data->event_1_calls_count == 100);
  assert(data->event_2_calls_count == 100);

  free(data);
}

static void event_1_0(
  common_data_t *common_data, void *usr_data,
  bt_bool cf_1, const char* cf_2, uint64_t cf_3, int64_t cf_4, bt_bool pf_1, const char* pf_2, uint64_t pf_3, int64_t pf_4)
{
  data_t *data = (data_t *) usr_data;
  data->event_1_calls_count += 1;
}

static void event_2_0(
  common_data_t *common_data, void *usr_data,
  bt_bool cf_1, const char* cf_2, uint64_t cf_3, int64_t cf_4, bt_bool pf_1, const char* pf_2, uint64_t pf_3, int64_t pf_4)
{
  data_t *data = (data_t *) usr_data;
  data->event_2_calls_count += 1;
}


void btx_register_usr_callbacks(name_to_dispatcher_t** name_to_dispatcher) {
  btx_register_callbacks_event_1(name_to_dispatcher, &event_1_0);
  btx_register_callbacks_event_2(name_to_dispatcher, &event_2_0);
}