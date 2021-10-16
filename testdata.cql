CREATE KEYSPACE IF NOT EXISTS projectData
WITH replication = {'class': 'SimpleStrategy', 'replication_factor' : 1};
	
CREATE TABLE IF NOT EXISTS projectData.methods
(
  method_hash UUID,
  projectID bigint,
  startVersionTime timestamp,
  file text,
  startVersionHash text,
  endVersionTime timestamp,
  endVersionHash text,
  name text,
  lineNumber int,
  authors set<UUID>,
  parserversion bigint,
  PRIMARY KEY ((method_hash), projectID, file, startVersionTime)
)
WITH CLUSTERING ORDER BY (projectID ASC, file ASC, startVersionTime DESC);
	
CREATE TABLE IF NOT EXISTS projectData.projects
(
  projectID bigint,
  versionTime timestamp,
  versionHash text,
  license text,
  name text,
  url text,
  ownerID UUID,
  hashes set<UUID>,
  parserversion bigint,
  PRIMARY KEY ((projectID), versionTime)
)
WITH CLUSTERING ORDER BY (versionTime DESC);

CREATE TABLE IF NOT EXISTS projectData.method_by_author
(
	authorID UUID,
	hash UUID,
	projectID bigint,
	file text,
	startVersionTime timestamp,  
	PRIMARY KEY ((authorID), hash, projectID, file, startVersionTime)
)
WITH CLUSTERING ORDER BY (hash ASC, projectID ASC, file ASC, startVersionTime DESC);

CREATE TABLE IF NOT EXISTS projectData.author_by_id
(
	authorID UUID,
	name text,
	mail text,
	PRIMARY KEY (authorID)
);

CREATE KEYSPACE IF NOT EXISTS jobs
WITH replication = {'class': 'NetworkTopologyStrategy', 'datacenter1' : 1};
	
CREATE TABLE IF NOT EXISTS jobs.jobsqueue 
(
	jobid UUID,
	priority bigint,
	url text,
	constant int,
	retries int,
	timeout bigint,
	PRIMARY KEY ((constant), priority, jobid)
)
WITH CLUSTERING ORDER BY (priority ASC);

CREATE TABLE IF NOT EXISTS jobs.currentjobs 
(
	jobid UUID,
	time timestamp,
	timeout bigint,
	priority bigint,
	url text,
	retries int,	
	PRIMARY KEY ((jobid)) 
);

CREATE TABLE IF NOT EXISTS jobs.failedjobs 
(
	jobid UUID,
	time timestamp,
	timeout bigint,
	priority bigint,
	url text,
	retries int,
	reasonID int,
	readonData text,
	PRIMARY KEY ((jobid), time) 
)
WITH CLUSTERING ORDER BY (time DESC);

CREATE TABLE IF NOT EXISTS jobs.variables 
(
	name text,
	value int,
	PRIMARY KEY (name)
);

INSERT INTO jobs.variables (name, value) VALUES ('crawlID', 0);

INSERT INTO projectData.methods (method_hash, projectID, startVersionTime, startVersionHash, endVersionTime, endVersionHash, name, file, lineNumber, authors, parserversion) VALUES 
(2c7f46d4-f57c-f9e6-6b03-213358c7ddb5, 1, 5000000000000, '9e350b124404f40a114509910619f641', 5000000000000, '9e350b124404f40a114509910619f641', 'M1', 'P1/M1.cpp', 1, 
{ 68bd2db6-fe91-47d2-a134-cf82b104f547 }, 1);
INSERT INTO projectData.methods (method_hash, projectID, startVersionTime, startVersionHash, endVersionTime, endVersionHash, name, file, lineNumber, authors, parserversion) VALUES 
(06f73d7a-b461-84c5-5bf4-742b9428a4c0, 2, 5000000001000, '9d075dfba5c2a903ff1f542ea729ae8b', 5000000001000, '9d075dfba5c2a903ff1f542ea729ae8b', 'M2', 'P2/M2.cpp', 1, 
{ 68bd2db6-fe91-47d2-a134-cf82b104f547, b2217c08-06eb-4a57-b977-7c6d72299301, 
  5a10bb4d-97b6-44d8-a135-f1432424a61c }, 1);
INSERT INTO projectData.methods (method_hash, projectID, startVersionTime, startVersionHash, endVersionTime, endVersionHash, name, file, lineNumber, authors, parserversion) VALUES 
(137fed01-7b61-59ac-c0af-30d2c6b403a5, 3, 5000000002000, '2d8b3b65caf0e9168a39be667be24861', 5000000004000, '8a04d1e679548a35a6feec48321cac81', 'M3', 'P3/M3.cpp', 1, 
{ b2217c08-06eb-4a57-b977-7c6d72299301 }, 1);
INSERT INTO projectData.methods (method_hash, projectID, startVersionTime, startVersionHash, endVersionTime, endVersionHash, name, file, lineNumber, authors, parserversion) VALUES 
(23920776-594c-85fd-c30c-d96f928487f1, 3, 5000000003000, 'e7b60fac745437880c1ccb8c4dd29f0f', 5000000004000, '8a04d1e679548a35a6feec48321cac81', 'M4', 'P3/M4.cpp', 21, 
{ 68bd2db6-fe91-47d2-a134-cf82b104f547, b2217c08-06eb-4a57-b977-7c6d72299301 }, 1);
INSERT INTO projectData.methods (method_hash, projectID, startVersionTime, startVersionHash, endVersionTime, endVersionHash, name, file, lineNumber, authors, parserversion) VALUES 
(959ee1ee-12e6-d6d8-7a4b-6ee732aed9fc, 3, 5000000004000, '8a04d1e679548a35a6feec48321cac81', 5000000004000, '8a04d1e679548a35a6feec48321cac81', 'M5', 'P3/M5.cpp', 41, 
{ 68bd2db6-fe91-47d2-a134-cf82b104f547, d7dd6d67-76ca-414e-85c2-59adad13bd23 }, 1);
INSERT INTO projectData.methods (method_hash, projectID, startVersionTime, startVersionHash, endVersionTime, endVersionHash, name, file, lineNumber, authors, parserversion) VALUES 
(06f73d7a-b461-84c5-5bf4-742b9428a4c0, 4, 5000000005000, '70966cd9481793ab85a409374a66f36b', 5000000006000, '6415e258c077e5cf3f98982d8050e941', 'M6', 'P4/M6.cpp', 3, 
{ e39e0872-6856-4fa0-8d9a-278728362f43 }, 1);
INSERT INTO projectData.methods (method_hash, projectID, startVersionTime, startVersionHash, endVersionTime, endVersionHash, name, file, lineNumber, authors, parserversion) VALUES 
(8811e6be-db87-e90c-ef39-de1179f3bd2e, 4, 5000000006000, '6415e258c077e5cf3f98982d8050e941', 5000000006000, '6415e258c077e5cf3f98982d8050e941', 'M7', 'P4/M7.cpp', 23, 
{ f95ffc6c-aa97-40d6-b709-cb4823955213 }, 1);
INSERT INTO projectData.methods (method_hash, projectID, startVersionTime, startVersionHash, endVersionTime, endVersionHash, name, file, lineNumber, authors, parserversion) VALUES 
(23920776-594c-85fd-c30c-d96f928487f1, 5, 5000000007000, 'e35706965bfee40184ae6b4f38c1c81d', 5000000009000, '63308e3dbf0aba36ecaf66d5f51b6d2b', 'M8', 'P5/M8.cpp', 1, 
{ e39e0872-6856-4fa0-8d9a-278728362f43, f95ffc6c-aa97-40d6-b709-cb4823955213 }, 1);
INSERT INTO projectData.methods (method_hash, projectID, startVersionTime, startVersionHash, endVersionTime, endVersionHash, name, file, lineNumber, authors, parserversion) VALUES 
(959ee1ee-12e6-d6d8-7a4b-6ee732aed9fc, 5, 5000000008000, '9120d38cee8bea61f5759d40a4b8e19e', 5000000009000, '63308e3dbf0aba36ecaf66d5f51b6d2b', 'M9', 'P5/M9.cpp', 41, 
{ f95ffc6c-aa97-40d6-b709-cb4823955213 }, 1);
INSERT INTO projectData.methods (method_hash, projectID, startVersionTime, startVersionHash, endVersionTime, endVersionHash, name, file, lineNumber, authors, parserversion) VALUES 
(8811e6be-db87-e90c-ef39-de1179f3bd2e, 5, 5000000009000, '63308e3dbf0aba36ecaf66d5f51b6d2b', 5000000009000, '63308e3dbf0aba36ecaf66d5f51b6d2b', 'M10', 'P5/M10.cpp', 61, 
{ 2a84cf5a-9554-4800-bb87-6dda6715fa12 }, 1);
INSERT INTO projectData.methods (method_hash, projectID, startVersionTime, startVersionHash, endVersionTime, endVersionHash, name, file, lineNumber, authors, parserversion) VALUES
(261a8712-b146-7bd2-6844-de6c43c9f8f1, 101, 5000000008000, '429ae78a6b15630c0ce5114d02b0c55f', 5000000009000, '8be58ce8426f941e1f856bf5e4e14492', 'M101', 'P101/M101.cpp', 1,
{ e39e0872-6856-4fa0-8d9a-278728362f43 }, 1);
INSERT INTO projectData.methods (method_hash, projectID, startVersionTime, startVersionHash, endVersionTime, endVersionHash, name, file, lineNumber, authors, parserversion) VALUES
(ee9ae09b-3a21-12f5-fd2a-63847b74920a, 101, 5000000009000, '8be58ce8426f941e1f856bf5e4e14492', 5000000009000, '8be58ce8426f941e1f856bf5e4e14492', 'M101', 'P101/M101.cpp', 41,
{ e39e0872-6856-4fa0-8d9a-278728362f43, f95ffc6c-aa97-40d6-b709-cb4823955213 }, 1);

INSERT INTO projectData.projects (projectID, versionTime, versionHash, license, name, url, ownerID, hashes, parserversion) VALUES 
(1, 5000000000000, '9e350b124404f40a114509910619f641', 'L1', 'P1', 'www.github.com/p1', 68bd2db6-fe91-47d2-a134-cf82b104f547, 
{ 2c7f46d4-f57c-f9e6-6b03-213358c7ddb5 }, 1);
INSERT INTO projectData.projects (projectID, versionTime, versionHash, license, name, url, ownerID, hashes, parserversion) VALUES 
(2, 5000000001000, '9d075dfba5c2a903ff1f542ea729ae8b', 'L2', 'P2', 'www.github.com/p2', 68bd2db6-fe91-47d2-a134-cf82b104f547, 
{ 06f73d7a-b461-84c5-5bf4-742b9428a4c0 }, 1);
INSERT INTO projectData.projects (projectID, versionTime, versionHash, license, name, url, ownerID, hashes, parserversion) VALUES 
(3, 5000000002000, '2d8b3b65caf0e9168a39be667be24861', 'L3', 'P3', 'www.github.com/p3', b2217c08-06eb-4a57-b977-7c6d72299301, 
{ 137fed01-7b61-59ac-c0af-30d2c6b403a5 }, 1);
INSERT INTO projectData.projects (projectID, versionTime, versionHash, license, name, url, ownerID, hashes, parserversion) VALUES 
(3, 5000000003000, 'e7b60fac745437880c1ccb8c4dd29f0f', 'L3', 'P3', 'www.github.com/p3', b2217c08-06eb-4a57-b977-7c6d72299301, 
{ 137fed01-7b61-59ac-c0af-30d2c6b403a5, 23920776-594c-85fd-c30c-d96f928487f1 }, 1);
INSERT INTO projectData.projects (projectID, versionTime, versionHash, license, name, url, ownerID, hashes, parserversion) VALUES 
(3, 5000000004000, '8a04d1e679548a35a6feec48321cac81', 'L3', 'P3', 'www.github.com/p3', b2217c08-06eb-4a57-b977-7c6d72299301, 
{ 137fed01-7b61-59ac-c0af-30d2c6b403a5, 23920776-594c-85fd-c30c-d96f928487f1, 959ee1ee-12e6-d6d8-7a4b-6ee732aed9fc }, 1);
INSERT INTO projectData.projects (projectID, versionTime, versionHash, license, name, url, ownerID, hashes, parserversion) VALUES 
(4, 5000000005000, '70966cd9481793ab85a409374a66f36b', 'L4', 'P4', 'www.github.com/p4', e39e0872-6856-4fa0-8d9a-278728362f43, 
{ 06f73d7a-b461-84c5-5bf4-742b9428a4c0 }, 1);
INSERT INTO projectData.projects (projectID, versionTime, versionHash, license, name, url, ownerID, hashes, parserversion) VALUES 
(4, 5000000006000, '6415e258c077e5cf3f98982d8050e941', 'L4', 'P4', 'www.github.com/p4', e39e0872-6856-4fa0-8d9a-278728362f43, 
{ 06f73d7a-b461-84c5-5bf4-742b9428a4c0, 8811e6be-db87-e90c-ef39-de1179f3bd2e }, 1);
INSERT INTO projectData.projects (projectID, versionTime, versionHash, license, name, url, ownerID, hashes, parserversion) VALUES 
(5, 5000000007000, 'e35706965bfee40184ae6b4f38c1c81d', 'L4', 'P5', 'www.github.com/p5', e39e0872-6856-4fa0-8d9a-278728362f43, 
{ 23920776-594c-85fd-c30c-d96f928487f1 }, 1);
INSERT INTO projectData.projects (projectID, versionTime, versionHash, license, name, url, ownerID, hashes, parserversion) VALUES 
(5, 5000000008000, '9120d38cee8bea61f5759d40a4b8e19e', 'L4', 'P5', 'www.github.com/p5', e39e0872-6856-4fa0-8d9a-278728362f43, 
{ 23920776-594c-85fd-c30c-d96f928487f1, 959ee1ee-12e6-d6d8-7a4b-6ee732aed9fc }, 1);
INSERT INTO projectData.projects (projectID, versionTime, versionHash, license, name, url, ownerID, hashes, parserversion) VALUES 
(5, 5000000009000, '63308e3dbf0aba36ecaf66d5f51b6d2b', 'L4', 'P5', 'www.github.com/p5', e39e0872-6856-4fa0-8d9a-278728362f43, 
{ 23920776-594c-85fd-c30c-d96f928487f1, 959ee1ee-12e6-d6d8-7a4b-6ee732aed9fc, 8811e6be-db87-e90c-ef39-de1179f3bd2e }, 1);
INSERT INTO projectData.projects (projectID, versionTime, versionHash, license, name, url, ownerID, hashes, parserversion) VALUES
(101, 5000000008000, '429ae78a6b15630c0ce5114d02b0c55f', 'L5', 'P101', 'www.github.com/p101', e39e0872-6856-4fa0-8d9a-278728362f43,
{ 261a8712-b146-7bd2-6844-de6c43c9f8f1 }, 1);
INSERT INTO projectData.projects (projectID, versionTime, versionHash, license, name, url, ownerID, hashes, parserversion) VALUES
(101, 5000000009000, '8be58ce8426f941e1f856bf5e4e14492', 'L5', 'P101', 'www.github.com/p101', e39e0872-6856-4fa0-8d9a-278728362f43,
{ 261a8712-b146-7bd2-6844-de6c43c9f8f1, ee9ae09b-3a21-12f5-fd2a-63847b74920a }, 1);

INSERT INTO projectData.method_by_author (authorID, hash, startVersionTime, projectID, file) VALUES
(68bd2db6-fe91-47d2-a134-cf82b104f547, 2c7f46d4-f57c-f9e6-6b03-213358c7ddb5, 5000000000000, 1, 'P1/M1.cpp');
INSERT INTO projectData.method_by_author (authorID, hash, startVersionTime, projectID, file) VALUES
(68bd2db6-fe91-47d2-a134-cf82b104f547, 06f73d7a-b461-84c5-5bf4-742b9428a4c0, 4999999999000, 2, 'P2/M2.cpp');
INSERT INTO projectData.method_by_author (authorID, hash, startVersionTime, projectID, file) VALUES
(b2217c08-06eb-4a57-b977-7c6d72299301, 06f73d7a-b461-84c5-5bf4-742b9428a4c0, 5000000000000, 2, 'P2/M2.cpp');
INSERT INTO projectData.method_by_author (authorID, hash, startVersionTime, projectID, file) VALUES
(5a10bb4d-97b6-44d8-a135-f1432424a61c, 06f73d7a-b461-84c5-5bf4-742b9428a4c0, 5000000001000, 2, 'P2/M2.cpp');
INSERT INTO projectData.method_by_author (authorID, hash, startVersionTime, projectID, file) VALUES
(b2217c08-06eb-4a57-b977-7c6d72299301, 137fed01-7b61-59ac-c0af-30d2c6b403a5, 5000000002000, 3, 'P3/M3.cpp');
INSERT INTO projectData.method_by_author (authorID, hash, startVersionTime, projectID, file) VALUES
(68bd2db6-fe91-47d2-a134-cf82b104f547, 23920776-594c-85fd-c30c-d96f928487f1, 5000000002000, 3, 'P3/M3.cpp');
INSERT INTO projectData.method_by_author (authorID, hash, startVersionTime, projectID, file) VALUES
(b2217c08-06eb-4a57-b977-7c6d72299301, 23920776-594c-85fd-c30c-d96f928487f1, 5000000003000, 3, 'P3/M4.cpp');
INSERT INTO projectData.method_by_author (authorID, hash, startVersionTime, projectID, file) VALUES
(68bd2db6-fe91-47d2-a134-cf82b104f547, 959ee1ee-12e6-d6d8-7a4b-6ee732aed9fc, 5000000003000, 3, 'P3/M4.cpp');
INSERT INTO projectData.method_by_author (authorID, hash, startVersionTime, projectID, file) VALUES
(d7dd6d67-76ca-414e-85c2-59adad13bd23, 959ee1ee-12e6-d6d8-7a4b-6ee732aed9fc, 5000000004000, 3, 'P3/M5.cpp');
INSERT INTO projectData.method_by_author (authorID, hash, startVersionTime, projectID, file) VALUES
(e39e0872-6856-4fa0-8d9a-278728362f43, 06f73d7a-b461-84c5-5bf4-742b9428a4c0, 5000000005000, 4, 'P4/M6.cpp');
INSERT INTO projectData.method_by_author (authorID, hash, startVersionTime, projectID, file) VALUES
(f95ffc6c-aa97-40d6-b709-cb4823955213, 8811e6be-db87-e90c-ef39-de1179f3bd2e, 5000000006000, 4, 'P4/M7.cpp');
INSERT INTO projectData.method_by_author (authorID, hash, startVersionTime, projectID, file) VALUES
(e39e0872-6856-4fa0-8d9a-278728362f43, 23920776-594c-85fd-c30c-d96f928487f1, 5000000006000, 5, 'P5/M7.cpp');
INSERT INTO projectData.method_by_author (authorID, hash, startVersionTime, projectID, file) VALUES
(f95ffc6c-aa97-40d6-b709-cb4823955213, 23920776-594c-85fd-c30c-d96f928487f1, 5000000007000, 5, 'P5/M8.cpp');
INSERT INTO projectData.method_by_author (authorID, hash, startVersionTime, projectID, file) VALUES
(f95ffc6c-aa97-40d6-b709-cb4823955213, 959ee1ee-12e6-d6d8-7a4b-6ee732aed9fc, 5000000008000, 5, 'P5/M9.cpp');
INSERT INTO projectData.method_by_author (authorID, hash, startVersionTime, projectID, file) VALUES
(2a84cf5a-9554-4800-bb87-6dda6715fa12, 8811e6be-db87-e90c-ef39-de1179f3bd2e, 5000000008000, 5, 'P5/M9.cpp');
INSERT INTO projectData.method_by_author (authorID, hash, startVersionTime, projectID, file) VALUES
(e39e0872-6856-4fa0-8d9a-278728362f43, 8811e6be-db87-e90c-ef39-de1179f3bd2e, 5000000009000, 5, 'P5/M10.cpp');
INSERT INTO projectData.method_by_author (authorID, hash, startVersionTime, projectID, file) VALUES
(e39e0872-6856-4fa0-8d9a-278728362f43, 261a8712-b146-7bd2-6844-de6c43c9f8f1, 5000000008000, 101, 'P101/M101.cpp');
INSERT INTO projectData.method_by_author (authorID, hash, startVersionTime, projectID, file) VALUES
(e39e0872-6856-4fa0-8d9a-278728362f43, ee9ae09b-3a21-12f5-fd2a-63847b74920a, 5000000009000, 101, 'P101/M101.cpp');
INSERT INTO projectData.method_by_author (authorID, hash, startVersionTime, projectID, file) VALUES
(f95ffc6c-aa97-40d6-b709-cb4823955213, ee9ae09b-3a21-12f5-fd2a-63847b74920a, 5000000009000, 101, 'P101/M101.cpp');
INSERT INTO projectData.method_by_author (authorID, hash, startVersionTime, projectID, file) VALUES
(41ab7373-8f24-4a03-83dc-621036d99f34, 137fed01-7b61-59ac-c0af-30d2c6b403a5, 420, 69, 'P69/M1.cpp');
INSERT INTO projectData.method_by_author (authorID, hash, startVersionTime, projectID, file) VALUES
(47919e8f-7103-48a3-9514-3f2d9d49ac61, 2c7f46d4-f57c-f9e6-6b03-213358c7ddb5, 69, 42, 'P42/M1.cpp');
INSERT INTO projectData.method_by_author (authorID, hash, startVersionTime, projectID, file) VALUES
(47919e8f-7103-48a3-9514-3f2d9d49ac61, 06f73d7a-b461-84c5-5bf4-742b9428a4c0, 420, 42, 'P42/M2.cpp');

INSERT INTO projectData.author_by_id (authorID, name, mail) VALUES 
(68bd2db6-fe91-47d2-a134-cf82b104f547, 'Author 1', 'author1@mail.com');
INSERT INTO projectData.author_by_id (authorID, name, mail) VALUES 
(b2217c08-06eb-4a57-b977-7c6d72299301, 'Author 2', 'author2@mail.com');
INSERT INTO projectData.author_by_id (authorID, name, mail) VALUES 
(5a10bb4d-97b6-44d8-a135-f1432424a61c, 'Author 3', 'author3@mail.com');
INSERT INTO projectData.author_by_id (authorID, name, mail) VALUES 
(d7dd6d67-76ca-414e-85c2-59adad13bd23, 'Author 4', 'author4@mail.com');
INSERT INTO projectData.author_by_id (authorID, name, mail) VALUES 
(e39e0872-6856-4fa0-8d9a-278728362f43, 'Author 5', 'author5@mail.com');
INSERT INTO projectData.author_by_id (authorID, name, mail) VALUES 
(f95ffc6c-aa97-40d6-b709-cb4823955213, 'Author 6', 'author6@mail.com');
INSERT INTO projectData.author_by_id (authorID, name, mail) VALUES 
(2a84cf5a-9554-4800-bb87-6dda6715fa12, 'Author 7', 'author7@mail.com');
INSERT INTO projectData.author_by_id (authorID, name, mail) VALUES 
(47919e8f-7103-48a3-9514-3f2d9d49ac61, 'Author1', 'author1@mail.com');
INSERT INTO projectData.author_by_id (authorID, name, mail) VALUES 
(41ab7373-8f24-4a03-83dc-621036d99f34, 'Author2', 'author2@mail.com');

INSERT INTO jobs.jobsqueue (jobid, priority, url, constant, retries, timeout) VALUES 
(5bcd0d08-ded0-4134-b165-1c7d0352c15e, 500000000000000, 'https://github.com/zavg/linux-0.01', 1, 0, 69);
INSERT INTO jobs.jobsqueue (jobid, priority, url, constant, retries, timeout) VALUES
(61c9d732-1f98-40af-b201-3b20fbf7a6db, 400000000000000, 'https://github.com/304471720/CSAPI', 1, 0, 42);
INSERT INTO jobs.jobsqueue (jobid, priority, url, constant, retries, timeout) VALUES
(2f78a799-0e18-4eb5-bd33-83718a3257d9, 300000000000000, 'https://github.com/caged/microsis', 1, 0, 100);
INSERT INTO jobs.jobsqueue (jobid, priority, url, constant, retries, timeout) VALUES
(77b02074-8a8e-4193-ac11-53a83ae2ad0d, 200000000000000, 'https://github.com/eladrich/pixel2style2pixel', 1, 0, 42069);
INSERT INTO jobs.jobsqueue (jobid, priority, url, constant, retries, timeout) VALUES
(6bb4b5bf-119b-4323-a175-469132620a24, 100000000000000, 'https://github.com/mattermost/desktop', 1, 0, 69420);