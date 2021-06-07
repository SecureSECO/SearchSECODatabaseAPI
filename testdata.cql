CREATE KEYSPACE IF NOT EXISTS projectData
    WITH replication = {'class': 'SimpleStrategy', 'replication_factor' : 1};

CREATE KEYSPACE IF NOT EXISTS jobs
    WITH replication = {'class': 'SimpleStrategy', 'replication_factor' : 1};
    
CREATE TABLE IF NOT EXISTS projectData.methods
(
  method_hash UUID,
  version timestamp,
  projectID bigint,
  name text,
  file text,
  lineNumber int,
  authors set<UUID>,
  PRIMARY KEY ((method_hash), version, projectID)
) 
WITH CLUSTERING ORDER BY (version DESC, projectID ASC);
    
CREATE TABLE IF NOT EXISTS projectData.projects
(
  projectID bigint,
  version timestamp,
  license text,
  name text,
  url text,
  ownerID UUID,
  hashes set<text>,
  PRIMARY KEY ((projectID), version)
) 
WITH CLUSTERING ORDER BY (version DESC);

CREATE TABLE IF NOT EXISTS projectData.method_by_author
(
  authorID UUID,
  hash UUID,
  version timestamp,
  projectID bigint,
  PRIMARY KEY ((authorID), hash, version, projectID)
)
WITH CLUSTERING ORDER BY (hash ASC, version DESC, projectID ASC);

CREATE TABLE IF NOT EXISTS projectData.author_by_id
(
  authorID UUID,
  name text,
  mail text,
  PRIMARY KEY (authorID)
);

CREATE TABLE IF NOT EXISTS projectData.id_by_author
(
  name text,
  mail text,
  authorID UUID,
  PRIMARY KEY ((name, mail))
);

CREATE TABLE IF NOT EXISTS jobs.jobsqueue 
(
    jobid UUID,
    priority int,
    url text,
    constant int,
    PRIMARY KEY ((constant), priority, jobid) 
) 
WITH CLUSTERING ORDER BY (priority DESC);

INSERT INTO projectData.methods (method_hash, projectID, version, name, file, lineNumber, authors) VALUES 
(2c7f46d4-f57c-f9e6-6b03-213358c7ddb5, 1, 5000000000000, 'M1', 'P1/M1.cpp', 1, 
{ 68bd2db6-fe91-47d2-a134-cf82b104f547 });
INSERT INTO projectData.methods (method_hash, projectID, version, name, file, lineNumber, authors) VALUES 
(06f73d7a-b461-84c5-5bf4-742b9428a4c0, 2, 5000000001000, 'M2', 'P2/M2.cpp', 1, 
{ 68bd2db6-fe91-47d2-a134-cf82b104f547, b2217c08-06eb-4a57-b977-7c6d72299301, 
  5a10bb4d-97b6-44d8-a135-f1432424a61c });
INSERT INTO projectData.methods (method_hash, projectID, version, name, file, lineNumber, authors) VALUES 
(137fed01-7b61-59ac-c0af-30d2c6b403a5, 3, 5000000002000, 'M3', 'P3/M3.cpp', 1, 
{ b2217c08-06eb-4a57-b977-7c6d72299301 });
INSERT INTO projectData.methods (method_hash, projectID, version, name, file, lineNumber, authors) VALUES 
(23920776-594c-85fd-c30c-d96f928487f1, 3, 5000000003000, 'M4', 'P3/M4.cpp', 21, 
{ 68bd2db6-fe91-47d2-a134-cf82b104f547, b2217c08-06eb-4a57-b977-7c6d72299301 });
INSERT INTO projectData.methods (method_hash, projectID, version, name, file, lineNumber, authors) VALUES 
(959ee1ee-12e6-d6d8-7a4b-6ee732aed9fc, 3, 5000000004000, 'M5', 'P3/M5.cpp', 41, 
{ 68bd2db6-fe91-47d2-a134-cf82b104f547, d7dd6d67-76ca-414e-85c2-59adad13bd23 });
INSERT INTO projectData.methods (method_hash, projectID, version, name, file, lineNumber, authors) VALUES 
(06f73d7a-b461-84c5-5bf4-742b9428a4c0, 4, 5000000005000, 'M6', 'P4/M6.cpp', 3, 
{ e39e0872-6856-4fa0-8d9a-278728362f43 });
INSERT INTO projectData.methods (method_hash, projectID, version, name, file, lineNumber, authors) VALUES 
(8811e6be-db87-e90c-ef39-de1179f3bd2e, 4, 5000000006000, 'M7', 'P4/M7.cpp', 23, 
{ f95ffc6c-aa97-40d6-b709-cb4823955213 });
INSERT INTO projectData.methods (method_hash, projectID, version, name, file, lineNumber, authors) VALUES 
(23920776-594c-85fd-c30c-d96f928487f1, 5, 5000000007000, 'M8', 'P5/M8.cpp', 1, 
{ e39e0872-6856-4fa0-8d9a-278728362f43, f95ffc6c-aa97-40d6-b709-cb4823955213 });
INSERT INTO projectData.methods (method_hash, projectID, version, name, file, lineNumber, authors) VALUES 
(959ee1ee-12e6-d6d8-7a4b-6ee732aed9fc, 5, 5000000008000, 'M9', 'P5/M9.cpp', 41, 
{ f95ffc6c-aa97-40d6-b709-cb4823955213 });
INSERT INTO projectData.methods (method_hash, projectID, version, name, file, lineNumber, authors) VALUES 
(8811e6be-db87-e90c-ef39-de1179f3bd2e, 5, 5000000009000, 'M10', 'P5/M10.cpp', 61, 
{ 2a84cf5a-9554-4800-bb87-6dda6715fa12 });
INSERT INTO projectData.methods (method_hash, projectID, version, name, file, lineNumber, authors) VALUES
(261a8712-b146-7bd2-6844-de6c43c9f8f1, 101, 5000000008000, 'P101', 'www.github.com/p101', 1,
{ e39e0872-6856-4fa0-8d9a-278728362f43 });
INSERT INTO projectData.methods (method_hash, projectID, version, name, file, lineNumber, authors) VALUES
(ee9ae09b-3a21-12f5-fd2a-63847b74920a, 101, 5000000009000, 'P101', 'www.github.com/p101', 41,
{ e39e0872-6856-4fa0-8d9a-278728362f43, f95ffc6c-aa97-40d6-b709-cb4823955213 });

INSERT INTO projectData.projects (projectID, version, license, name, url, ownerID, hashes) VALUES 
(1, 5000000000000, 'L1', 'P1', 'www.github.com/p1', 68bd2db6-fe91-47d2-a134-cf82b104f547, 
{ '2c7f46d4f57cf9e66b03213358c7ddb5' });
INSERT INTO projectData.projects (projectID, version, license, name, url, ownerID, hashes) VALUES 
(2, 5000000001000, 'L2', 'P2', 'www.github.com/p2', 68bd2db6-fe91-47d2-a134-cf82b104f547, 
{ '06f73d7ab46184c55bf4742b9428a4c0' });
INSERT INTO projectData.projects (projectID, version, license, name, url, ownerID, hashes) VALUES 
(3, 5000000002000, 'L3', 'P3', 'www.github.com/p3', b2217c08-06eb-4a57-b977-7c6d72299301, 
{ '137fed017b6159acc0af30d2c6b403a5', '23920776594c85fdc30cd96f928487f1', '959ee1ee12e6d6d87a4b6ee732aed9fc' });
INSERT INTO projectData.projects (projectID, version, license, name, url, ownerID, hashes) VALUES 
(4, 5000000005000, 'L4', 'P4', 'www.github.com/p4', e39e0872-6856-4fa0-8d9a-278728362f43, 
{ '06f73d7ab46184c55bf4742b9428a4c0', '8811e6bedb87e90cef39de1179f3bd2e' });
INSERT INTO projectData.projects (projectID, version, license, name, url, ownerID, hashes) VALUES 
(5, 5000000007000, 'L4', 'P5', 'www.github.com/p5', e39e0872-6856-4fa0-8d9a-278728362f43, 
{ '23920776594c85fdc30cd96f928487f1', '959ee1ee12e6d6d87a4b6ee732aed9fc', '8811e6bedb87e90cef39de1179f3bd2e' });
INSERT INTO projectData.projects (projectID, version, license, name, url, ownerID, hashes) VALUES
(101, 5000000008000, 'L5', 'P101', 'www.github.com/p101', e39e0872-6856-4fa0-8d9a-278728362f43,
{ '261a8712b1467bd26844de6c43c9f8f1' });
INSERT INTO projectData.projects (projectID, version, license, name, url, ownerID, hashes) VALUES
(101, 5000000009000, 'L5', 'P101', 'www.github.com/p101', e39e0872-6856-4fa0-8d9a-278728362f43,
{ 'ee9ae09b3a2112f5fd2a63847b74920a' });

INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(68bd2db6-fe91-47d2-a134-cf82b104f547, 2c7f46d4-f57c-f9e6-6b03-213358c7ddb5, 5000000000000, 1);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(68bd2db6-fe91-47d2-a134-cf82b104f547, 06f73d7a-b461-84c5-5bf4-742b9428a4c0, 4999999999000, 2);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(b2217c08-06eb-4a57-b977-7c6d72299301, 06f73d7a-b461-84c5-5bf4-742b9428a4c0, 5000000000000, 2);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(5a10bb4d-97b6-44d8-a135-f1432424a61c, 06f73d7a-b461-84c5-5bf4-742b9428a4c0, 5000000001000, 2);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(b2217c08-06eb-4a57-b977-7c6d72299301, 137fed01-7b61-59ac-c0af-30d2c6b403a5, 5000000002000, 3);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(68bd2db6-fe91-47d2-a134-cf82b104f547, 23920776-594c-85fd-c30c-d96f928487f1, 5000000002000, 3);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(b2217c08-06eb-4a57-b977-7c6d72299301, 23920776-594c-85fd-c30c-d96f928487f1, 5000000003000, 3);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(68bd2db6-fe91-47d2-a134-cf82b104f547, 959ee1ee-12e6-d6d8-7a4b-6ee732aed9fc, 5000000003000, 3);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(d7dd6d67-76ca-414e-85c2-59adad13bd23, 959ee1ee-12e6-d6d8-7a4b-6ee732aed9fc, 5000000004000, 3);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(e39e0872-6856-4fa0-8d9a-278728362f43, 06f73d7a-b461-84c5-5bf4-742b9428a4c0, 5000000005000, 4);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(f95ffc6c-aa97-40d6-b709-cb4823955213, 8811e6be-db87-e90c-ef39-de1179f3bd2e, 5000000006000, 4);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(e39e0872-6856-4fa0-8d9a-278728362f43, 23920776-594c-85fd-c30c-d96f928487f1, 5000000006000, 5);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(f95ffc6c-aa97-40d6-b709-cb4823955213, 23920776-594c-85fd-c30c-d96f928487f1, 5000000007000, 5);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(f95ffc6c-aa97-40d6-b709-cb4823955213, 959ee1ee-12e6-d6d8-7a4b-6ee732aed9fc, 5000000008000, 5);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(2a84cf5a-9554-4800-bb87-6dda6715fa12, 8811e6be-db87-e90c-ef39-de1179f3bd2e, 5000000008000, 5);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(e39e0872-6856-4fa0-8d9a-278728362f43, 8811e6be-db87-e90c-ef39-de1179f3bd2e, 5000000009000, 5);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(e39e0872-6856-4fa0-8d9a-278728362f43, 261a8712-b146-7bd2-6844-de6c43c9f8f1, 5000000008000, 101);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(e39e0872-6856-4fa0-8d9a-278728362f43, ee9ae09b-3a21-12f5-fd2a-63847b74920a, 5000000009000, 101);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(f95ffc6c-aa97-40d6-b709-cb4823955213, ee9ae09b-3a21-12f5-fd2a-63847b74920a, 5000000009000, 101);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(41ab7373-8f24-4a03-83dc-621036d99f34, 137fed01-7b61-59ac-c0af-30d2c6b403a5, 420, 69);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(47919e8f-7103-48a3-9514-3f2d9d49ac61, 2c7f46d4-f57c-f9e6-6b03-213358c7ddb5, 69, 42);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(47919e8f-7103-48a3-9514-3f2d9d49ac61, 06f73d7a-b461-84c5-5bf4-742b9428a4c0, 420, 42);

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

INSERT INTO projectData.id_by_author (authorID, name, mail) VALUES
(68bd2db6-fe91-47d2-a134-cf82b104f547, 'Author 1', 'author1@mail.com');
INSERT INTO projectData.id_by_author (authorID, name, mail) VALUES 
(b2217c08-06eb-4a57-b977-7c6d72299301, 'Author 2', 'author2@mail.com');
INSERT INTO projectData.id_by_author (authorID, name, mail) VALUES 
(5a10bb4d-97b6-44d8-a135-f1432424a61c, 'Author 3', 'author3@mail.com');
INSERT INTO projectData.id_by_author (authorID, name, mail) VALUES 
(d7dd6d67-76ca-414e-85c2-59adad13bd23, 'Author 4', 'author4@mail.com');
INSERT INTO projectData.id_by_author (authorID, name, mail) VALUES 
(e39e0872-6856-4fa0-8d9a-278728362f43, 'Author 5', 'author5@mail.com');
INSERT INTO projectData.id_by_author (authorID, name, mail) VALUES 
(f95ffc6c-aa97-40d6-b709-cb4823955213, 'Author 6', 'author6@mail.com');
INSERT INTO projectData.author_by_id (authorID, name, mail) VALUES 
(2a84cf5a-9554-4800-bb87-6dda6715fa12, 'Author 7', 'author7@mail.com');

INSERT INTO jobs.jobsqueue (jobid, priority, url, constant) VALUES 
(5bcd0d08-ded0-4134-b165-1c7d0352c15e, 3, 'https://github.com/zavg/linux-0.01', 1);
INSERT INTO jobs.jobsqueue (jobid, priority, url, constant) VALUES
(61c9d732-1f98-40af-b201-3b20fbf7a6db, 4, 'https://github.com/304471720/CSAPI', 1);
INSERT INTO jobs.jobsqueue (jobid, priority, url, constant) VALUES
(2f78a799-0e18-4eb5-bd33-83718a3257d9, 5, 'https://github.com/caged/microsis', 1);

INSERT INTO projectData.id_by_author (authorID, name, mail) VALUES 
(2a84cf5a-9554-4800-bb87-6dda6715fa12, 'Author 7', 'author7@mail.com');
INSERT INTO projectData.id_by_author (authorID, name, mail) VALUES 
(47919e8f-7103-48a3-9514-3f2d9d49ac61, 'Author1', 'author1@mail.com');
INSERT INTO projectData.id_by_author (authorID, name, mail) VALUES 
(41ab7373-8f24-4a03-83dc-621036d99f34, 'Author2', 'author2@mail.com');

