------------------------------------------------------------------------------
------------------------------------------------------------------------------
-- Cheddar is a GNU GPL real-time scheduling analysis tool.
-- This program provides services to automatically check schedulability and
-- other performance criteria of real-time architecture models.
--
-- Copyright (C) 2002-2020, Frank Singhoff, Alain Plantec, Jerome Legrand,
--                          Hai Nam Tran, Stephane Rubini
--
-- The Cheddar project was started in 2002 by
-- Frank Singhoff, Lab-STICC UMR 6285, Universit√© de Bretagne Occidentale
--
-- Cheddar has been published in the "Agence de Protection des Programmes/France" in 2008.
-- Since 2008, Ellidiss technologies also contributes to the development of
-- Cheddar and provides industrial support.
--
-- The full list of contributors and sponsors can be found in AUTHORS.txt and SPONSORS.txt
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
--
--
-- Contact : cheddar@listes.univ-brest.fr
--
------------------------------------------------------------------------------
-- Last update :
--    $Rev: 3657 $
--    $Date: 2020-12-13 13:25:49 +0100 (dim., 13 d√©c. 2020) $
--    $Author: singhoff $
------------------------------------------------------------------------------
------------------------------------------------------------------------------

with xml_tag;           use xml_tag;
with double_util;       use double_util;
with translate;         use translate;
with unbounded_strings; use unbounded_strings;
with systems;           use systems;
with Ada.Tags;          use Ada.Tags;
with Text_IO;           use Text_IO;
with debug;             use debug;
with batteries;         use batteries;
with sets;

package body scheduler.dynamic_priority.edh is

   procedure initialize (a_scheduler : in out edh_scheduler) is
   begin
      reset (a_scheduler);
      a_scheduler.parameters.scheduler_type :=
        earliest_deadline_first_protocol;
   end initialize;

   function copy
     (a_scheduler : in edh_scheduler) return generic_scheduler_ptr
   is
      ptr : edh_scheduler_ptr;

   begin

      ptr := new edh_scheduler;

      ptr.parameters         := a_scheduler.parameters;
      ptr.previously_elected := a_scheduler.previously_elected;

      return generic_scheduler_ptr (ptr);

   end copy;

   procedure do_election
     (my_scheduler       : in out edh_scheduler;
      si                 : in out scheduling_information;
      result             : in out scheduling_sequence_ptr;
      msg                : in out Unbounded_String;
      current_time       : in     Natural;
      processor_name     : in     Unbounded_String;
      address_space_name : in     Unbounded_String;
      core_name          : in     Unbounded_String;
      options            : in     scheduling_option;
      event_to_generate  : in     time_unit_event_type_boolean_table;
      elected            : in out tasks_range;
      no_task            : in out Boolean)

   is

      smallest_deadline        : Natural     := Natural'last;
      i                        : tasks_range := 0;
      k                        : tasks_range := 0;
      j                        : tasks_range := 0;
      is_ready                 : Boolean     := False;
      previous_task_can_be_run : Boolean     := False;
      -- Battery
      battery_ok       : Boolean := False;
      battery_iterator : batteries_iterator;
      the_battery      : battery_ptr;
      -- Processor
      processor_is_idle : Boolean := True;
      -- Slack time
      slack_time : Natural := Natural'last;
      stti       : Natural := 0;
      hi         : Natural := 0;
      ati        : Natural := 0;
      inter      : Natural := 0;
      -- Slack energy
      slack_energy : Natural := Natural'last;
      seti         : Natural := 0;
      ep           : Natural := 0;
      gi           : Natural := 0;

   begin

      put_debug ("Call Do_Election: EDH");

      -- Test on the battery
      if (not is_empty (si.batteries)) then

         reset_iterator (si.batteries, battery_iterator);
         loop
            current_element (si.batteries, the_battery, battery_iterator);

            if (the_battery.cpu_name = processor_name) then
               battery_ok := True;
               if (current_time = 0) then
                  my_scheduler.energy := the_battery.initial_energy;
               end if;
            end if;

            exit when is_last_element (si.batteries, battery_iterator);

            next_element (si.batteries, battery_iterator);
         end loop;

      end if;
      -- ...

      put_debug
        (" My_scheduler.Energy : " &
         my_scheduler.energy'img &
         " at : " &
         current_time'img);

      -- If the battery's cpu name corresponds to the processor then we can execute the procedure
      -- For now, we consider that there can only be one battery
      if (battery_ok) then

         -- Job Set Slack Time at Current_Time
         loop

            if si.tcbs (k) /= null then

               if (si.tcbs (k).tsk.cpu_name = processor_name) then
                  dynamic_priority_tcb_ptr (si.tcbs (k)).dynamic_deadline :=
                    si.tcbs (k).wake_up_time + si.tcbs (k).tsk.deadline;
               end if;

               hi   := 0;
               ati  := 0;
               stti := 0;

               --
               loop

                  if si.tcbs (j) /= null then
                     if
                       (dynamic_priority_tcb_ptr (si.tcbs (j))
                          .dynamic_deadline <=
                        dynamic_priority_tcb_ptr (si.tcbs (k))
                          .dynamic_deadline)
                     then

               -- Hi : The processor demand of a job set at time Current_Time
                        hi := hi + si.tcbs (j).tsk.capacity;

                        -- ATi : Total remaining execution time for uncompleted jobs at time Current_Time
                        ati := ati + si.tcbs (j).rest_of_capacity;

                     end if;

                  end if;

                  j := j + 1;
                  exit when si.tcbs (j) = null;

               end loop;
               -- ...

               -- STti : Slack time of a job at time Current_Time
               inter := hi - ati;
               stti  :=
                 Natural'max
                   (0,
                    dynamic_priority_tcb_ptr (si.tcbs (k)).dynamic_deadline -
                    current_time -
                    inter);

               -- ST : Slack time of a job set at time Current_Time
               if (stti < slack_time) and
                 (dynamic_priority_tcb_ptr (si.tcbs (k)).dynamic_deadline >
                  current_time)
               then
                  slack_time := stti;
               end if;

            end if;

            j := 0;

            k := k + 1;
            exit when si.tcbs (k) = null;

         end loop;
         -- End of Job Set Slack Time at Current_Time

         -- Job Set Slack Energy at Current_Time
         if (si.tcbs (my_scheduler.previously_elected) /= null) then

            k := 0;
            j := 0;

            loop

               if si.tcbs (k) /= null then

                  gi := 0;
                  ep := 0;

                  --
                  loop

                     if si.tcbs (j) /= null then
                        if
                          ((current_time <= si.tcbs (j).wake_up_time) and
                           (dynamic_priority_tcb_ptr (si.tcbs (j))
                              .dynamic_deadline <=
                            dynamic_priority_tcb_ptr (si.tcbs (k))
                              .dynamic_deadline))
                        then

                        -- Gi : Energy demand of a job set at time Current_Time
                           gi := gi + si.tcbs (j).tsk.energy_consumption;

                        end if;

                     end if;

                     j := j + 1;
                     exit when si.tcbs (j) = null;

                  end loop;
                  -- ...

         -- Ep : Energy produced in the time interval (Current_Time,Deadline)
                  ep :=
                    Natural'max
                      (0,
                       (dynamic_priority_tcb_ptr (si.tcbs (k))
                          .dynamic_deadline -
                        current_time -
                        si.tcbs (k).tsk.start_time) *
                       the_battery.rechargeable_power);

                  -- SEti : Slack energy of a job at time Current_Time
                  seti := Natural'max (0, my_scheduler.energy + ep - gi);

                  -- PSE : Slack energy of the job set at time Current_Time
                  if (seti < slack_energy) then
                     if
                       ((current_time < si.tcbs (k).wake_up_time) and
                        (si.tcbs (k).wake_up_time <
                         dynamic_priority_tcb_ptr (si.tcbs (k))
                           .dynamic_deadline) and
                        (dynamic_priority_tcb_ptr (si.tcbs (k))
                           .dynamic_deadline <
                         dynamic_priority_tcb_ptr
                           (si.tcbs (my_scheduler.previously_elected))
                           .dynamic_deadline))
                     then
                        slack_energy := seti;
                     end if;

                  end if;
               end if;

               j := 0;

               k := k + 1;
               exit when si.tcbs (k) = null;

            end loop;
         end if;

         if (slack_energy < the_battery.e_max) then
            slack_energy := 0;
         end if;
         -- End of Job Set Slack Energy at Current_Time

         put_debug ("ST " & slack_time'img);
         put_debug ("PSE " & slack_energy'img);

         loop

            if not si.tcbs (i).already_run_at_current_time then

               if (si.tcbs (i).tsk.cpu_name = processor_name) then

                  if check_core_assignment (my_scheduler, si.tcbs (i)) then

                     if (si.tcbs (i).wake_up_time <= current_time) and
                       (si.tcbs (i).rest_of_capacity /= 0)
                     then

                        if options.with_resources then

                           check_resource
                             (my_scheduler,
                              si,
                              result,
                              current_time,
                              si.tcbs (i),
                              is_ready,
                              event_to_generate);

                        else
                           is_ready := True;
                        end if;

                        if is_ready then

                           check_jitter
                             (si.tcbs (i),
                              current_time,
                              si.tcbs (i).is_jitter_ready);
                           if (options.with_jitters = False) or
                             (si.tcbs (i).is_jitter_ready)
                           then

                              if (options.with_offsets = False) or
                                check_offset (si.tcbs (i), current_time)
                              then

                                 if (options.with_precedencies = False) or
                                   check_precedencies
                                     (si,
                                      current_time,
                                      si.tcbs (i))
                                 then

                                    if i = my_scheduler.previously_elected then
                                       previous_task_can_be_run := True;
                                    end if;

                                    -- Rule n∞1 : EDF priority
                                    if
                                      (dynamic_priority_tcb_ptr (si.tcbs (i))
                                         .dynamic_deadline <
                                       smallest_deadline)
                                    then
                                       smallest_deadline :=
                                         dynamic_priority_tcb_ptr (si.tcbs (i))
                                           .dynamic_deadline;
                                       elected := i;
                                    end if;
                                    -- End of rule n∞1

                                    -- Rule n∞3 :
                                    if (my_scheduler.energy = 0) or
                                      (my_scheduler.energy <
                                       si.tcbs (i).tsk.capacity) or
                                      (slack_energy = 0)
                                    then
                                       processor_is_idle := True;
                                    end if;
                                    -- End of rule n∞3

                                    -- Rule n∞4 :
                                    if
                                      (my_scheduler.energy =
                                       the_battery.capacity) or
                                      (slack_time = 0)
                                    then
                                       processor_is_idle := False;
                                    end if;
                                    -- End of rule n∞4

                                    -- Rule n∞5
                                    if (my_scheduler.energy > 0) and
                                      (my_scheduler.energy <
                                       the_battery.capacity) and
                                      (my_scheduler.energy >=
                                       si.tcbs (i).tsk.capacity) and
                                      (slack_time > 0) and
                                      (slack_energy > 0)
                                    then
                                       -- Processor can equally be idle or busy
                                       -- For now it is busy
                                       processor_is_idle := False;
                                    end if;
                                    -- End of rule n∞5

                                 end if;
                              end if;
                           end if;
                        end if;
                     end if;
                  end if;
               end if;
            end if;

            i := i + 1;
            exit when si.tcbs (i) = null;
         end loop;

         -- Rule n∞2 :
         if smallest_deadline = Natural'last then
            processor_is_idle := True;
         end if;
         -- End of rule n∞2

         --
         if processor_is_idle = True then
            no_task             := True;
            my_scheduler.energy :=
              my_scheduler.energy + the_battery.rechargeable_power;
            if my_scheduler.energy > the_battery.capacity then
               my_scheduler.energy := the_battery.capacity;
            end if;
         elsif processor_is_idle = False then
            no_task := False;
            if
              (my_scheduler.energy -
               si.tcbs (elected).tsk.energy_consumption +
               the_battery.rechargeable_power *
                 si.tcbs (elected).tsk.capacity <
               0)
            then
               my_scheduler.energy := 0;
            else
               my_scheduler.energy :=
                 my_scheduler.energy -
                 si.tcbs (elected).tsk.energy_consumption +
                 the_battery.rechargeable_power *
                   si.tcbs (elected).tsk.capacity;
            end if;
            if my_scheduler.energy > the_battery.capacity then
               my_scheduler.energy := the_battery.capacity;
            end if;
         end if;

         -- By default, as task are sorted in the set according to their name
         -- when we have two tasks with the same absolute deadline, we choose the first one
         -- in the task set, i.e. the task with the smallest name.
         -- This strategy can be useful has it provides a simple mean to introduce a
         -- tie break as a kind of fixed priority.
         -- However, it may introduce an extra preemption.
         -- If we want to reduce preemption number as much as possible, in this case
         -- we select the previous task ... in this task can be run again !
         --
         if options.with_minimize_preemption and previous_task_can_be_run then
            if dynamic_priority_tcb_ptr
                (si.tcbs (my_scheduler.previously_elected))
                .dynamic_deadline =
              smallest_deadline
            then
               elected := my_scheduler.previously_elected;
               put_debug ("Call Do_Election: EDF : Minimize preemption");
            end if;
         end if;

         put_debug ("Call Do_Election: EDH : Elected : " & elected'img);

      end if;

   end do_election;

end scheduler.dynamic_priority.edh;
