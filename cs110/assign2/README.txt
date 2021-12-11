All of these optimizations were designed to reduce the number of reads that went
to disk. All other factors were insignificant compared to the performance hit of
disk latency, so I focused my attention on these as opposed to run-time
optimizations. There were no obvious back of the envelope calculations, since
they all involved caching and this is unpredictable depending on the ordering of
sector access, file access, etc. Assuming that each sector is read sequentially
(which they are not), there would be a reduction of runtime by a factor of
512. All optimizations were measured by how much they reduced IO operations.

Optimization one: Sector-level cache
------------------------------------
I implemented a simple sector cache that was designed to prefer keeping adjacent
sectors in memory at the same time: by using the sector number modulo the total
number of available spaces, the most recently accessed sectors that tend to have
close sector numbers are available in the cache. I considered also adding
preloading of nearby sectors, but since this would involve another call to the
low-level read function and can't be done in the background, it would not speed
things up. After this optimization, the number of IO operations for vlarge were
1.33% of their original value and 0.06% of their original value for large.

Optimization two: Fileops
-------------------------
The largest improvement in this area was reducing the number of calls made by
getchar to the lower-level inode and file functions, in conjunction with a small
sector cache. This was accomplished by storing information about the file
instead of fetching it every time a character was read and by storing the
current block of the current file in memory to prevent having to fetch it for
every single character read. The cache prevents having to load an entire sector
for every character. The number of IO operations once again dropped
considerably. The number of IOs for large were 42.2% of their previous values
and the number for vlarge was 4.82% of its previous value.

Optimization three: Checksumming
--------------------------------
Instead of computing the checksum every time it was needed to determine if two
files were identical, the checksum is computed when the file is loaded into the
Pathstore and accessed from memory thereafter. This prevents having to re-read
the file from disk every single time the program needs to check if it's
identical to another file, which is done every time a new file is added to the
Pathstore. By deferring checksumming until it's actually required as opposed to
as soon as the file is loaded, performance for single-file disks is preserved.
The IO operations for large did not change (no negative hit) and the values for
vlarge dropped to 5.05% of their previous values. After this optimization, all
values achieve 100% score as given in the assignment description.


vlarge performance:
-------------------
-16kB: 130433 IO reads
-256kB: 39020 IO reads
-512kB: 30287 IO reads

For all, user time was around 4.75s and system time was around 0.05s
