### Solution 2 
### Definitions
#### Global Variables 
- int waitingStudents - Stores the number of currently waiting students
- int numCompanies, numZones, numStudents - m, n, o
- int vaccineUsed[MAX] - Stores the number of batches of each company used
- struct vaccineData vaccineQueue[QUEUE_MAX] - The companies put the manufactured vaccines into this queue
- int readPosVaccine, writePosVaccine - The variables to read and write to queue
- int slotQueue[MAX] - Stores which zone has how many slots remaining
- int zoneStudent[MAX][MAX] - Maps which Zone has which students assigned
- bool vaccinated[MAX] - Which students have been vaccinated yet
- double vaccineGiven[MAX] - What was success prob of vaccine given to the student

#### Mutex Locks
- vaccineQueueLock - Synchronizes the access to vaccineQueue, vaccineUsed among Zones, Companies
- slotQueueLock - Synchronizes the access to waitingStudents, zoneStudent, slotQueue among Students, Zones
- studentLock - Synchronizes access to vaccinated, vaccineGiven

#### Conditional Variables
- vaccineNotUsed (vaccineQueueLock) Companies wait on this while their medicine is being used
- noVaccineAvailable (slotQueueLock) Zones wait on this if no vaccine
- noSlotsAvailable (slotQueueLock) Students wait on this if no slot available
- notVaccinated (studentLock) Student wait on this when assigned but not vaccinated

### Working
#### Companies 
1. When it is created it gets an ID and pSuccess
2. ```addToVaccineQueue ``` makes vaccines and adds them to queue
3. Then company waits on ```vaccineNotUsed``` and checks ```vaccineUsed[companyId]```
4. When all of its batches are used it again goes ```step 1```

#### Zones
1. When it is created it gets an ID
2. Initially it has 0 vaccines from no company
3. While it has 0 vaccines it calls ``` getFromVaccineQueue``` and checks vaccine
4. ``` getFromVaccineQueue``` waits on ```noVaccineAvailable``` until there are some vaccines in ```vaccinesQueue```
5. When it gets some vaccines it returns them to the Zone
6. Now Zone will call ```addToSlotsQueue``` which will add slots for the zone in ```slotsQueue```
7. After that Zone will ```sleep``` so that students can register
8. Zone will take a note of registered students and zone will remove its slots from ```slotqueue``` to prepare for 
vaccination phase. If no students registered control goes to ```step 6```
9. Zone will vaccinate the registered students in vaccination zone
10. Then it signals students waiting on ```notVaccinated```
11. If the vaccines are zero it goes to ```step 3``` or else it goes to ```step 6```

#### Students
1. They enter collegeGate and ```sleep``` for random time to get ready for vaccination
2. They call ```getRegistered``` to get registered to a Zone for vaccination
3. If there are no zones with slots it waits on ```noSlotsAvailable```
4. After getting registered student waits on ```notVaccinated``` until the Zone vaccinates them
5. Then they are tested for antibodies according to the ```pSuccess``` of the vaccination
6. If they have antibodies student thread ```exit```
7. If they dont have antibodies they can get registered again in ```Step 2``` upto maximum 3 times in total.

#### Main
1. It takes input
2. Creates all the threads and gives ID in ascending order
3. Waits for student threads to exit