# File-Management-System
File Management System (FMS) library has functionalities to handle a file, i.e., Create, Open,
Close, Write, Delete, Find, FindNext, Re-index. FMS creates two files behind this system, which are
data file (.dat), and indexing file (.ndx). Data file consists of two parts; header and record. Header
part is taken from “header.json” file by json parsing. Header part holds general
information about file. “recordLength” is the size of a record. “keyStart” and “keyEnd” are the
starting and ending bytes of key (inclusive in the end) that is used for indexing. “order”
holds the direction of the sorting (ASC: ascending, DESC: descending). Besides, a new
parameter called “open” must be added to header part, which represents the current status of
the file (true=open and false=closed).
