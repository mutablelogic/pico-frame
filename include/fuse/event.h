/** @file event.h
 *  @brief Fuse events
 *
 *  This file defines events which are placed on the event queues for the application.
 */
#ifndef FUSE_EVENT_H
#define FUSE_EVENT_H

#include "value.h"

// Define the event types
#define FUSE_EVENT_NULL 0x00   ///< NULL event
#define FUSE_EVENT_TIMER 0x01  ///< Timer event
#define FUSE_EVENT_GPIO 0x02   ///< GPIO input event
#define FUSE_EVENT_PWM 0x03    ///< PWM level event
#define FUSE_EVENT_ADC 0x04    ///< ADC samples event
#define FUSE_EVENT_SPI_TX 0x05 ///< SPI transmit event
#define FUSE_EVENT_SPI_RX 0x06 ///< SPI receive event
#define FUSE_EVENT_BME280 0x07 ///< BME280 measurement event

// Maximum number of events
#define FUSE_EVENT_COUNT 0x08       ///< Maximum number of events
#define FUSE_EVENT_CALLBACK_COUNT 3 ///< Maximum number of callbacks per event

#ifdef DEBUG
#define fuse_new_event(self, source, type, data) \
    ((fuse_event_t *)fuse_new_event_ex((self), (source), (type), (data), __FILE__, __LINE__))
#else
#define fuse_new_event(self, source, type, data) \
    ((fuse_event_t *)fuse_new_event_ex((self), (source), (type), (data), 0, 0))
#endif

//////////////////////////////////////////////////////////////////////////////
// DECLARATIONS

#include <stdint.h>
#include <stdbool.h>
#include "value.h"

/** @brief An opaque event object
 */
typedef struct event_context fuse_event_t;

/** @brief Place a new event on the event queues
 *
 * An event is created and placed on all event queues for the application.
 * The event is retained by each event queue.
 *
 * @param self The fuse instance
 * @param source The source of the event
 * @param type The event type
 * @param user_data User data associated with the event, if any
 * @return The event is returned, or NULL if the event could not be created
 */
fuse_event_t *fuse_new_event_ex(fuse_t *self, fuse_value_t *source, uint8_t type, void *user_data, const char *file, const int line);

/** @brief Retrieve an event from the event queue
 *
 * An event is retrieved from an event queue for the application. The event is released
 * from the queue.
 *
 * @param self The fuse instance
 * @param q The queue to retrieve the event from (0 or 1)
 * @return The event is returned, or NULL if there is no event in the queue
 */
fuse_event_t *fuse_next_event(fuse_t *self, uint8_t q);

/** @brief Callback for an event
 */
typedef void (*fuse_callback_t)(fuse_t *self, fuse_event_t *event, void *user_data);

/** @brief Register a callback for an event
 *
 * An event is retrieved from an event queue for the application. The event is released
 * from the queue.
 *
 * @param self The fuse instance
 * @param q The cpre to implement the callback execution for (0 or 1)
 * @param type The event type
 * @return Returns true if the callback was registered, otherwise false
 */
bool fuse_register_callback(fuse_t *self, uint8_t type, uint8_t q, fuse_callback_t callback);

/** @brief Execute callbacks for an event
 *
 * Callbacks for an event are called in order.
 *
 * @param self The fuse instance
 * @param q The queue to retrieve the event callbacks from (0 or 1)
 * @param evt The event to execute
 */
void fuse_exec_event(fuse_t *self, uint8_t q, fuse_event_t *event);

/** @brief Return the event source
 *
 * Returns the source of an event. If the event is NULL, then NULL is returned.
 *
 * @param self The fuse application
 * @param event The event
 * @returns The source of the event
 */
fuse_value_t *fuse_event_source(fuse_t *self, fuse_event_t *event);

#endif /* FUSE_EVENT_H */
