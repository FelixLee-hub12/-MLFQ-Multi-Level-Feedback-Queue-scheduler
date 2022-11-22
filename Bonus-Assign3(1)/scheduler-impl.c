#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

void outprint(int time_x, int time_y, int pid, int arrival_time, int remaining_time);
void bubbleSortProcessArray(Process proc[], int n);
int FindProc(Process proc[], int proc_num, Process p);
void scheduler(Process *proc, LinkedQueue **ProcessQueue, int proc_num, int queue_num, int period)
{
    /* printf("Process number: %d\n", proc_num);
    for (int i = 0;i < proc_num; i++)
        printf("%d %d %d\n", proc[i].process_id, proc[i].arrival_time, proc[i].execution_time);

    printf("\nQueue number: %d\n", queue_num);
    printf("Period: %d\n", period);
    for (int i = 0;i < queue_num; i++){
        printf("%d %d %d\n", i, ProcessQueue[i]->time_slice, ProcessQueue[i]->allotment_time);
    }

    /*
       Test outprint function, it will output "Time_slot:1-2, pid:3, arrival-time:4, remaining_time:5" to output.loc file.
    */
    // outprint(1,2,3,4,5); */
    int tmp_time = 0;
    int time_slice[queue_num];
    int process_allotment_limit[proc_num]; // count the remaining allotment slide of each process
    int remaining_time[proc_num];

    for (int i = 0; i < proc_num; i++)
    {
        remaining_time[i] = proc[i].execution_time;
    }

    for (int i = 0; i < queue_num; i++)
    {
        time_slice[i] = ProcessQueue[i]->time_slice;
    }
    Process de_proc;
    int currentQueue = queue_num - 1; // initialize the current queue

    do
    {
        int finish_flag = 1;
        for (int i = 0; i < proc_num; i++){
            if(remaining_time[i] > 0){
                finish_flag = 0;
                break;
            }
        }
        if (finish_flag){
            break;
        }
        
        int dequeue_flag = 0;
        for (int i = 0; i < proc_num; i++)
        {
            //handle enqueue
            if (tmp_time == proc[i].arrival_time)
            {
                ProcessQueue[queue_num - 1] = EnQueue(ProcessQueue[queue_num - 1], proc[i]);
                process_allotment_limit[i] = ProcessQueue[queue_num - 1]->allotment_time; // assigns max allotment to that process
            }
            //handle dequeue
            if (remaining_time[i] == 0)
            { 
                de_proc = DeQueue(ProcessQueue[currentQueue]);
                dequeue_flag = 1;
            }
            
            //process uses up to whole slide
            if (process_allotment_limit[i] % time_slice[currentQueue] == 0 && process_allotment_limit[i] != ProcessQueue[currentQueue]->allotment_time){
                de_proc = DeQueue(ProcessQueue[currentQueue]);
                dequeue_flag = 1;
            }
        }

        if (dequeue_flag)
        {
            // lower the level of deleted process
            int de_index = FindProc(proc, proc_num, de_proc);
            if (remaining_time[de_index] > 0)
            {
                if (process_allotment_limit[de_index] == 0)
                {
                    process_allotment_limit[de_index] = ProcessQueue[currentQueue - 1]->allotment_time; // update the allotment time
                    ProcessQueue[currentQueue - 1] = EnQueue(ProcessQueue[currentQueue - 1], de_proc);  // enqueue to the lower level queue
                    proc[de_index].execution_time -= time_slice[currentQueue];
                    outprint(tmp_time - time_slice[currentQueue], tmp_time, de_proc.process_id, de_proc.arrival_time, remaining_time[de_index]);
                }
                // allotment limit > 0
                else
                {
                    ProcessQueue[currentQueue] = EnQueue(ProcessQueue[currentQueue], de_proc);
                    proc[de_index].execution_time -= time_slice[currentQueue];
                    outprint(tmp_time - time_slice[currentQueue], tmp_time, de_proc.process_id, de_proc.arrival_time, remaining_time[de_index]);
                }
            }
            if (remaining_time[de_index] == 0)
            {
                // deleted process does not use up to whole slice
                if (proc[de_index].execution_time < time_slice[currentQueue])
                {
                    outprint(tmp_time - proc[de_index].execution_time, tmp_time, de_proc.process_id, de_proc.arrival_time, 0);
                }
                else
                {
                    outprint(tmp_time - time_slice[currentQueue], tmp_time, de_proc.process_id, de_proc.arrival_time, 0);
                }
            }
            // reset currentQueue
            for (int i = queue_num - 1; i >= 0; i--)
            {
                if (!IsEmptyQueue(ProcessQueue[i]))
                {
                    currentQueue = i;
                    break;
                }
            }
            //handle all queues are emtpy
            if (IsEmptyQueue(ProcessQueue[currentQueue])){
                currentQueue = queue_num - 1;
            }
        }
        // implementation of period
        if (tmp_time >= period && tmp_time % period == 0)
        {
            // clear all queues
            Process extract_proc;
            int total_proc_inqueue = 0;
            for (int i = 0; i < queue_num; i++)
            {
                total_proc_inqueue += Length(ProcessQueue[i]);
            }

            Process tempProc[total_proc_inqueue];
            int index = 0;

            // delete all process in every queue and store it into tempProc
            for (int i = 0; i < queue_num; i++)
            {
                while (!IsEmptyQueue(ProcessQueue[i]))
                {
                    extract_proc = DeQueue(ProcessQueue[i]);
                    tempProc[index] = extract_proc;
                    index++;
                }
            }
            // sort ascendingly
            bubbleSortProcessArray(tempProc, total_proc_inqueue);
            // restore the process into first priority queue
            for (int i = 0; i < total_proc_inqueue; i++)
            {
                ProcessQueue[queue_num - 1] = EnQueue(ProcessQueue[queue_num - 1], tempProc[i]);
                // reassign the process allotment slide
                for (int j = 0; j < proc_num; j++)
                {
                    if (tempProc[i].process_id == proc[j].process_id)
                    {
                        process_allotment_limit[j] = ProcessQueue[queue_num - 1]->allotment_time;
                        break;
                    }
                }
            }
            currentQueue = queue_num - 1;
        }

        
        if (currentQueue == queue_num - 1){
            if (IsEmptyQueue(ProcessQueue[queue_num - 1])){}
            else {
                Process front_proc = FrontQueue(ProcessQueue[queue_num - 1]);
                int index = FindProc(proc, proc_num, front_proc);
                remaining_time[index] -= 1;
                process_allotment_limit[index] -= 1;
            }
        }
        // currentQueue != queue_num
        else{
            Process front_proc = FrontQueue(ProcessQueue[currentQueue]);
            int index = FindProc(proc, proc_num, front_proc);
            remaining_time[index] -= 1;
            process_allotment_limit[index] -= 1;
        }

    } while (++tmp_time);
}

void bubbleSortProcessArray(Process proc[], int n)
{
    for (int i = 0; i < n - 1; i++)
    {
        for (int j = 0; j < n - i - 1; j++)
        {
            if (proc[j].process_id > proc[j + 1].process_id)
            {
                Process temp = proc[j];
                proc[j] = proc[j + 1];
                proc[j + 1] = temp;
            }
        }
    }
}

int FindProc(Process proc[], int proc_num, Process p)
{
    for (int i = 0; i < proc_num; i++)
    {
        if (proc[i].process_id == p.process_id)
        {
            return i;
        }
    }
    return -1;
}