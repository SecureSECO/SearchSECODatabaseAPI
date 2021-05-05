CREATE KEYSPACE IF NOT EXISTS projectData
    WITH replication = {'class': 'SimpleStrategy', 'replication_factor' : 1};
    
CREATE TABLE IF NOT EXISTS projectData.methods
(
  method_hash text,
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
  hash text,
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

INSERT INTO projectData.methods (method_hash, projectID, version, name, file, lineNumber, authors) VALUES 
('2c7f46d4f57cf9e66b03213358c7ddb5', 1, 5000000000000, 'M1', 'P1/M1.cpp', 1, 
{ 68bd2db6-fe91-47d2-a134-cf82b104f547 });
INSERT INTO projectData.methods (method_hash, projectID, version, name, file, lineNumber, authors) VALUES 
('06f73d7ab46184c55bf4742b9428a4c0', 2, 5000000001000, 'M2', 'P2/M2.cpp', 1, 
{ 68bd2db6-fe91-47d2-a134-cf82b104f547, b2217c08-06eb-4a57-b977-7c6d72299301, 
  5a10bb4d-97b6-44d8-a135-f1432424a61c });
INSERT INTO projectData.methods (method_hash, projectID, version, name, file, lineNumber, authors) VALUES 
('137fed017b6159acc0af30d2c6b403a5', 3, 5000000002000, 'M3', 'P3/M3.cpp', 1, 
{ b2217c08-06eb-4a57-b977-7c6d72299301 });
INSERT INTO projectData.methods (method_hash, projectID, version, name, file, lineNumber, authors) VALUES 
('23920776594c85fdc30cd96f928487f1', 3, 5000000003000, 'M4', 'P3/M4.cpp', 21, 
{ 68bd2db6-fe91-47d2-a134-cf82b104f547, b2217c08-06eb-4a57-b977-7c6d72299301 });
INSERT INTO projectData.methods (method_hash, projectID, version, name, file, lineNumber, authors) VALUES 
('959ee1ee12e6d6d87a4b6ee732aed9fc', 3, 5000000004000, 'M5', 'P3/M5.cpp', 41, 
{ 68bd2db6-fe91-47d2-a134-cf82b104f547, d7dd6d67-76ca-414e-85c2-59adad13bd23 });
INSERT INTO projectData.methods (method_hash, projectID, version, name, file, lineNumber, authors) VALUES 
('06f73d7ab46184c55bf4742b9428a4c0', 4, 5000000005000, 'M6', 'P4/M6.cpp', 3, 
{ e39e0872-6856-4fa0-8d9a-278728362f43 });
INSERT INTO projectData.methods (method_hash, projectID, version, name, file, lineNumber, authors) VALUES 
('8811e6bedb87e90cef39de1179f3bd2e', 4, 5000000006000, 'M7', 'P4/M7.cpp', 23, 
{ f95ffc6c-aa97-40d6-b709-cb4823955213 });
INSERT INTO projectData.methods (method_hash, projectID, version, name, file, lineNumber, authors) VALUES 
('23920776594c85fdc30cd96f928487f1', 5, 5000000007000, 'M8', 'P5/M8.cpp', 1, 
{ e39e0872-6856-4fa0-8d9a-278728362f43, f95ffc6c-aa97-40d6-b709-cb4823955213 });
INSERT INTO projectData.methods (method_hash, projectID, version, name, file, lineNumber, authors) VALUES 
('959ee1ee12e6d6d87a4b6ee732aed9fc', 5, 5000000008000, 'M9', 'P5/M9.cpp', 41, 
{ f95ffc6c-aa97-40d6-b709-cb4823955213 });
INSERT INTO projectData.methods (method_hash, projectID, version, name, file, lineNumber, authors) VALUES 
('8811e6bedb87e90cef39de1179f3bd2e', 5, 5000000009000, 'M10', 'P5/M10.cpp', 61, 
{ 2a84cf5a-9554-4800-bb87-6dda6715fa12 });

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

INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(68bd2db6-fe91-47d2-a134-cf82b104f547, '2c7f46d4f57cf9e66b03213358c7ddb5', 5000000000000, 1);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(68bd2db6-fe91-47d2-a134-cf82b104f547, '06f73d7ab46184c55bf4742b9428a4c0', 4999999999000, 2);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(b2217c08-06eb-4a57-b977-7c6d72299301, '06f73d7ab46184c55bf4742b9428a4c0', 5000000000000, 2);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(5a10bb4d-97b6-44d8-a135-f1432424a61c, '06f73d7ab46184c55bf4742b9428a4c0', 5000000001000, 2);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(b2217c08-06eb-4a57-b977-7c6d72299301, '137fed017b6159acc0af30d2c6b403a5', 5000000002000, 3);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(68bd2db6-fe91-47d2-a134-cf82b104f547, '23920776594c85fdc30cd96f928487f1', 5000000002000, 3);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(b2217c08-06eb-4a57-b977-7c6d72299301, '23920776594c85fdc30cd96f928487f1', 5000000003000, 3);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(68bd2db6-fe91-47d2-a134-cf82b104f547, '959ee1ee12e6d6d87a4b6ee732aed9fc', 5000000003000, 3);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(d7dd6d67-76ca-414e-85c2-59adad13bd23, '959ee1ee12e6d6d87a4b6ee732aed9fc', 5000000004000, 3);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(e39e0872-6856-4fa0-8d9a-278728362f43, '06f73d7ab46184c55bf4742b9428a4c0', 5000000005000, 4);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(f95ffc6c-aa97-40d6-b709-cb4823955213, '8811e6bedb87e90cef39de1179f3bd2e', 5000000006000, 4);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(e39e0872-6856-4fa0-8d9a-278728362f43, '23920776594c85fdc30cd96f928487f1', 5000000006000, 5);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(f95ffc6c-aa97-40d6-b709-cb4823955213, '23920776594c85fdc30cd96f928487f1', 5000000007000, 5);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(f95ffc6c-aa97-40d6-b709-cb4823955213, '959ee1ee12e6d6d87a4b6ee732aed9fc', 5000000008000, 5);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(2a84cf5a-9554-4800-bb87-6dda6715fa12, '8811e6bedb87e90cef39de1179f3bd2e', 5000000008000, 5);
INSERT INTO projectData.method_by_author (authorID, hash, version, projectID) VALUES
(e39e0872-6856-4fa0-8d9a-278728362f43, '8811e6bedb87e90cef39de1179f3bd2e', 5000000009000, 5);

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

INSERT INTO projectData.id_by_author (authorID, name, mail) VALUES
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