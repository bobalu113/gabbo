/**
 * A module for tracking objects/programs as they are loaded and destructed.
 *
 * @author devo@eotl
 * @alias ObjectTracker
 */

inherit TrackerMixin;

// object info
({ object_name, object_time, program_name, program_time, destruct_time, last_ref_time, gigaticks, ticks })

// objects
([ object_name#object_time : info ])

// program info
({ program_name, program_time, size })

// programs
([ program_name#program_time : info ])

SQLClient
SQLClientFactory
