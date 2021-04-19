CREATE KEYSPACE IF NOT EXISTS projectData
    WITH replication = {'class': 'SimpleStrategy', 'replication_factor' : 1};
    
CREATE TABLE IF NOT EXISTS projectData.methods (
  method_hash text,
  version timestamp,
  projectID bigint,
  name text,
  file text,
  lineNumber int,
  authors set<UUID>,
  PRIMARY KEY ((method_hash), version, projectID)
) WITH CLUSTERING ORDER BY (version DESC, projectID ASC);
    
CREATE TABLE IF NOT EXISTS projectData.projects (
  projectID bigint,
  version timestamp,
  license text,
  name text,
  url text,
  ownerID UUID,
  hashes set<text>,
  PRIMARY KEY ((projectID), version)
 ) WITH CLUSTERING ORDER BY (version DESC)
;

CREATE TABLE IF NOT EXISTS projectData.method_by_author (
  authorID UUID,
  hash text,
  version timestamp,
  projectID bigint,
  PRIMARY KEY ((authorID), hash, version, projectID)
)  WITH CLUSTERING ORDER BY (hash ASC, version DESC, projectID ASC)
;

CREATE TABLE IF NOT EXISTS projectData.author_by_id (
  authorID UUID,
  name text,
  mail text,
  PRIMARY KEY (authorID)
);

CREATE TABLE IF NOT EXISTS projectData.id_by_author (
  name text,
  mail text,
  authorID UUID,
  PRIMARY KEY ((name, mail))
);