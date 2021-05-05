/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include "DatabaseHandler.h"

#define PROJECT_DATA_SIZE	7
#define METHOD_DATA_MIN_SIZE	5
#define HEX_CHARS		"0123456789abcdef"
/// <summary>
/// The different types of requests which are supported.
/// </summary>
enum eRequestType
{
	eUpload,
	eCheck,
	eCheckUpload,
	eUnknown
};

/// <summary>
/// Handles requests towards database.
/// </summary>
class RequestHandler
{
public:
	/// <summary>
	/// Makes the RequestHandler ready for later usage.
	/// </summary>
	/// <param name="databaseHandler">
	/// Handler for interactions with the database.
	/// </param>
	void initialize(DatabaseHandler* databaseHandler);

	/// <summary>
	/// Handles all requests send to the database.
	/// </summary>
	/// <param name="requestType">
	/// Type of the request, a string of exactly 4 characters.
	/// </param>
	/// <param name="request">
	/// The request made by the user, a string containing all
	/// relevant data in a specific order to be able to do the request.
	/// </param>
	/// <returns>
	/// Response towards user after processing the request successfully.
	/// </returns>
	std::string handleRequest(std::string requestType, std::string request);
private:
	/// <summary>
	/// Handles requests which want to add one project with their corresponding methods to the database.
	/// </summary>
	/// <param name="request">
	/// The request made by the user. It has the following format:
	/// "projectID?version?license?project_name?url?author_name?author_mail?stars \n
	///  method1_hash?method1_name?method1_fileLocation?method1_lineNumber?method1_numberOfAuthors?
	///  method1_author1_name?method1_author1_mail? <other authors> \n <method2_data> \n ... <methodN_data>".
	/// </param>
	/// <returns>
	/// Response towards user after processing the request successfully.
	/// </returns>
	std::string handleUploadRequest(std::string request);

	/// <summary>
	/// Converts a request to a Project (defined in Types.h).
	/// </summary>
	/// <param name="request">
	/// The relevant data to create the Project. It has the following format:
	/// "projectID?version?license?project_name?url?author_name?author_mail?stars".
	/// </param>
	/// <returns>
	/// A Project containing all data as provided within request.
	/// </returns>
	Project requestToProject(std::string request);

	/// <summary>
	/// Converts a data entry to a Method (defined in Types.h).
	/// </summary>
	/// <param name="dataEntry">
	/// The relevant data to create the Method. It has the following format:
	/// "method_hash?method_name?method_fileLocation?number_of_authors?
	///  method_author1_name?method_author1_mail?...?method_authorN_name?method_authorN_mail".
	/// </param>
	/// <returns>
	/// A Project containing all data as provided within request.
	/// </returns>
	MethodIn dataEntryToMethod(std::string dataEntry);

	/// <summary>
	/// Retrieves the hashes within a request.
	/// </summary>
	/// <param name="request">
	/// Represents the request made by the user. It has the following format:
	/// "projectID?version?license?project_name?url?author_name?author_mail?stars \n
	///  method1_hash?method1_name?method1_fileLocation?method1_lineNumber?method1_numberOfAuthors?
	///  method1_author1_name?method1_author1_mail? <other authors> \n <method2_data> \n ... <methodN_data>".
	/// </param>
	/// <returns>
	/// The hashes of the methods given in the requests.
	/// </returns>
	std::vector<Hash> requestToHashes(std::string request);

	/// <summary>
	/// Checks if a hash is valid.
	/// </summary>
	/// <param name = "hash">
	/// The hash to be checked.
	/// </param>
	/// <returns>
	/// Boolean indicating the validity of the given hash.
	/// </returns>
	bool isValidHash(Hash hash);

	/// <summary>
	/// Appends a vector of chars 'result' by methods which still need to be converted to vectors of chars.
	/// Also separates different methods and different method data elements by special characters,
	/// 'dataDelimiter' and 'methodDelimiters' respectively.
	/// </summary>
	/// <returns>
	/// A Project containing all data as provided within request.
	/// </returns>
	std::string methodsToString(std::vector<MethodOut> methods, char dataDelimiter, char methodDelimiter);

	/// <summary>
	/// Retrieves the methods corresponding to the hashes given as input using the database.
	/// </summary>
	/// <param name="hashes">
	/// A vector of hashes.
	/// </param>
	/// <returns>
	/// All methods in the database with a hash equal to one in 'hashes'.
	/// </returns>
	std::vector<MethodOut> getMethods(std::vector<Hash> hashes);

	/// <summary>
	/// Handles requests wanting to obtain methods with certain hashes.
	/// </summary>
	/// <param name="request">
	/// The request made by the user, having the following format:
	/// "hash_1\nhash_2\n...\nhash_N".
	/// </param>
	/// <returns>
	/// The methods which contain hashes equal to one within the request, in string format.
	/// </returns>
	std::string handleCheckRequest(std::string request);

	/// <summary>
	/// Handles requests wanting to obtain methods with certain hashes.
	/// </summary>
	/// <param name="hashes">
	/// The list of hashes we want to check for.
	/// </param>
	/// <returns>
	/// The methods which contain hashes equal to one within the request, in string format.
	/// </returns>
	std::string handleCheckRequest(std::vector<Hash> hashes);

	/// <summary>
	/// Handles requests wanting to first check for matches with existing methods in other projects,
	/// after which it adds the project itself to the database.
	/// </summary>
	/// <param name="request">
	/// The request made by the user, having the following format:
	/// "projectID?version?license?project_name?url?author_name?author_mail?stars\n
	///  method1_hash?method1_name?method1_fileLocation?method1_lineNumber?method1_numberOfAuthors?
	///  method1_author1_name?method1_author1_mail?...?method1_authorM_name?method1_authorM_mail\n
	///  <method2_data>\n...\n<methodN_data>".
	/// </param>
	/// <returns>
	/// The methods which contain hashes equal to the one of the hashes
	/// of the methods within the request, in string format.
	/// </returns>
	std::string handleCheckUploadRequest(std::string request);

	/// <summary>
	/// Handles unknown requests.
	/// </summary>
	/// <returns>
	/// A message telling the user that their input is not recognised.
	/// </returns>
	std::string handleUnknownRequest();

        /// <summary>
        /// Handles not implemented requests.
        /// </summary>
        /// <returns>
        /// A message telling the user that their input is not implemented yet.
        /// </returns>
        std::string handleNotImplementedRequest();

	/// <summary>
	/// Converts a requestType into an eRequestType.
	/// </summary>
	/// <param name="requestType">
	/// The type of the request, which is a string of exactly 4 characters.
	/// </param>
	/// <returns>
	/// A corresponding eRequestType.
	/// </returns>
	eRequestType getERequestType(std::string requestType);

	DatabaseHandler *database;
};
