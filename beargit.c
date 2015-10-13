#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>

#include "beargit.h"
#include "util.h"

/* Implementation Notes:
 *
 * - Functions return 0 if successful, 1 if there is an error.
 * - All error conditions in the function description need to be implemented
 *   and written to stderr. We catch some additional errors for you in main.c.
 * - Output to stdout needs to be exactly as specified in the function description.
 * - Only edit this file (beargit.c)
 * - Here are some of the helper functions from util.h:
 *   * fs_mkdir(dirname): create directory <dirname>
 *   * fs_rm(filename): delete file <filename>
 *   * fs_mv(src,dst): move file <src> to <dst>, overwriting <dst> if it exists
 *   * fs_cp(src,dst): copy file <src> to <dst>, overwriting <dst> if it exists
 *   * write_string_to_file(filename,str): write <str> to filename (overwriting contents)
 *   * read_string_from_file(filename,str,size): read a string of at most <size> (incl.
 *     NULL character) from file <filename> and store it into <str>. Note that <str>
 *     needs to be large enough to hold that string.
 *  - You NEED to test your code. The autograder we provide does not contain the
 *    full set of tests that we will run on your code. See "Step 5" in the project spec.
 */

/* beargit init
 *
 * - Create .beargit directory
 * - Create empty .beargit/.index file
 * - Create .beargit/.prev file containing 0..0 commit id
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_init(void) {
  fs_mkdir(".beargit");

  FILE* findex = fopen(".beargit/.index", "w");
  fclose(findex);

  FILE* fbranches = fopen(".beargit/.branches", "w");
  fprintf(fbranches, "%s\n", "master");
  fclose(fbranches);

  write_string_to_file(".beargit/.prev", "0000000000000000000000000000000000000000");
  write_string_to_file(".beargit/.current_branch", "master");

  return 0;
}



/* beargit add <filename>
 *
 * - Append filename to list in .beargit/.index if it isn't in there yet
 *
 * Possible errors (to stderr):
 * >> ERROR:  File <filename> has already been added.
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_add(const char* filename) {
  FILE* findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0) {
      fprintf(stderr, "ERROR:  File %s has already been added.\n", filename);
      fclose(findex);
      fclose(fnewindex);
      fs_rm(".beargit/.newindex");
      return 3;
    }

    fprintf(fnewindex, "%s\n", line);
  }

  fprintf(fnewindex, "%s\n", filename);
  fclose(findex);
  fclose(fnewindex);

  fs_mv(".beargit/.newindex", ".beargit/.index");

  return 0;
}

/* beargit status
 *
 * See "Step 1" in the project spec.
 *
 */

int beargit_status() {
  /* COMPLETE THE REST */
  char myData[FILENAME_SIZE*100];
  int count = 0;
  FILE* findex = fopen(".beargit/.index", "r");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
  	strcat(myData, line);
    count++;
  }
  
  fclose(findex);
  fprintf(stdout, "Tracked files:\n\n%s\nThere are %d files total.\n", myData, count);

  return 0;
}

/* beargit rm <filename>
 *
 * See "Step 2" in the project spec.
 *
 */

int beargit_rm(const char* filename) {
  /* COMPLETE THE REST */
  FILE* findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");

  int found = 0;

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0){
    	found = 1;
    	continue;
    }
    fprintf(fnewindex, "%s\n", line);
  }

  if (found == 0) {
    fprintf(stderr, "ERROR:  File %s not tracked.\n", filename);
    fclose(findex);
    fclose(fnewindex);
    fs_rm(".beargit/.newindex");
    return 1;
  }

  fclose(findex);
  fclose(fnewindex);
  fs_mv(".beargit/.newindex", ".beargit/.index");

  return 0;
}

/* beargit commit -m <msg>
 *
 * See "Step 3" in the project spec.
 *
 */

const char* go_bears = "THIS IS BEAR TERRITORY!";

int is_commit_msg_ok(const char* msg) {
  /* COMPLETE THE REST */
  const char* pter = go_bears;
  const char* msgpter = msg;

  while(*msgpter != '\0'){
  	if(*msgpter != *pter){
  		pter = go_bears;
  	}
  	else{
  		if (*pter == '!'){
        	return 1;
  		}
  		pter = pter + 1;
  	}
  	msgpter = msgpter + 1;
  }
  return 0;
}

/* Use next_commit_id to fill in the rest of the commit ID.
 *
 * Hints:
 * You will need a destination string buffer to hold your next_commit_id, before you copy it back to commit_id
 * You will need to use a function we have provided for you.
 */

void next_commit_id(char* commit_id) {
     /* COMPLETE THE REST */
  char buffer_ID[COMMIT_ID_SIZE];

  char currBranch[BRANCHNAME_SIZE + COMMIT_ID_SIZE + 1];
  read_string_from_file(".beargit/.current_branch", currBranch, BRANCHNAME_SIZE);

  strcat(currBranch, commit_id);
  cryptohash(currBranch, buffer_ID);
  strcpy(commit_id, buffer_ID);
  
}

int beargit_commit(const char* msg) {
  if (!is_commit_msg_ok(msg)) {
    fprintf(stderr, "ERROR:  Message must contain \"%s\"\n", go_bears);
    return 1;
  }

  char commit_id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE); 
  next_commit_id(commit_id);

    /* COMPLETE THE REST */
  char currBranch[BRANCHNAME_SIZE];

  read_string_from_file(".beargit/.current_branch", currBranch, BRANCHNAME_SIZE);
  if (strcmp(currBranch, "") == 0) {
    fprintf(stderr, "ERROR:  Need to be on HEAD of a branch to commit.\n");
    return 1;
  }

  char dirname[FILENAME_SIZE] = ".beargit/"; 
  strcat(dirname, commit_id);
  fs_mkdir(dirname);
  
  char dirindexname[FILENAME_SIZE] = "";
  strcat(dirindexname, dirname);
  strcat(dirindexname, "/.index");
 	
  char dirprevname[FILENAME_SIZE] = "";

  strcat(dirprevname, dirname);
  strcat(dirprevname, "/.prev");

  fs_cp(".beargit/.index", dirindexname);
  fs_cp(".beargit/.prev", dirprevname);

  FILE* findex = fopen(".beargit/.index", "r");
  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
	  strtok(line, "\n");
  	  char tempName[FILENAME_SIZE] = "";
  	  strcat(tempName, dirname);
  	  strcat(tempName, "/");
  	  strcat(tempName, line);
      
      fs_cp(line, tempName);
  }

  fclose(findex);

  char* fmsgname = dirname;
  strcat(fmsgname, "/.msg");
  write_string_to_file(fmsgname, msg);

  write_string_to_file(".beargit/.prev", commit_id);

  return 0;
}

/* beargit log
 *
 * See "Step 4" in the project spec.
 *
 */

void printLogInfo(char* ID, char* msg){
	char toPrint[MSG_SIZE + COMMIT_ID_SIZE + 100] = "commit ";
	strcat(toPrint, ID);
	strcat(toPrint, "\n   ");
	strcat(toPrint, msg);
	strcat(toPrint, "\n");
	fprintf(stdout, "%s\n", toPrint);
}

void getmsg(char* commit_id, char RTNMsg[MSG_SIZE]){
	char path[FILENAME_SIZE] = ".beargit/";
	strcat(path, commit_id);
	strcat(path, "/.msg");
	read_string_from_file(path, RTNMsg ,FILENAME_SIZE);
}

void getPrev(char* commit_id, char RTNID[COMMIT_ID_SIZE]){
	char path[FILENAME_SIZE] = ".beargit/";
	strcat(path, commit_id);
	strcat(path, "/.prev");
	read_string_from_file(path, RTNID ,FILENAME_SIZE);
}

int beargit_log(int limit) {
  /* COMPLETE THE REST */
	int index = limit;
	char currID[COMMIT_ID_SIZE] = "";
	char path[FILENAME_SIZE] = ".beargit/";
	strcat(path, "/.prev");
	read_string_from_file(path, currID ,FILENAME_SIZE);
	
	if (strcmp(currID, "0000000000000000000000000000000000000000") == 0){
		fprintf(stderr, "ERROR:  There are no commits.\n");
		return 1;
	}

	while(index > 0 && strcmp(currID, "0000000000000000000000000000000000000000") != 0){
		char currMSG[MSG_SIZE] = "";
		getmsg(currID, currMSG);	
		printLogInfo(currID, currMSG);
		getPrev(currID, currID);
		index--;
	}

  return 0;
}


// This helper function returns the branch number for a specific branch, or
// returns -1 if the branch does not exist.
int get_branch_number(const char* branch_name) {
  FILE* fbranches = fopen(".beargit/.branches", "r");

  int branch_index = -1;
  int counter = 0;
  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), fbranches)) {
    strtok(line, "\n");
    if (strcmp(line, branch_name) == 0) {
      branch_index = counter;
    }
    counter++;
  }

  fclose(fbranches);

  return branch_index;
}

/* beargit branch
 *
 * See "Step 5" in the project spec.
 *
 */

void printBranchInfo(char branchName[BRANCHNAME_SIZE], int OneIfCurr){
	char toPrint[BRANCHNAME_SIZE] = "";
	if (OneIfCurr == 1){
		strcat(toPrint, "*  ");
	} 
  else {
    strcat(toPrint, "   ");
  }
	strcat(toPrint, branchName);
	fprintf(stdout, "%s\n", toPrint);
}

int beargit_branch() {
  /* COMPLETE THE REST */

  char currBranch[BRANCHNAME_SIZE] = "";
  read_string_from_file(".beargit/.current_branch", currBranch ,BRANCHNAME_SIZE);
  
  FILE* fbranches = fopen(".beargit/.branches", "r");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), fbranches)) {
    strtok(line, "\n");
    if (strcmp(line, currBranch) == 0) {
      printBranchInfo(line, 1);
    } else{
      printBranchInfo(line, 0);
    }
  }

  fclose(fbranches);

  return 0;
}

/* beargit checkout
 *
 * See "Step 6" in the project spec.
 *
 */

int checkout_commit(const char* commit_id) {
  /* COMPLETE THE REST */
  FILE* findex = fopen(".beargit/.index", "r");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    fs_rm(line);
  }
  fclose(findex);

  if (strcmp(commit_id, "0000000000000000000000000000000000000000") != 0) {
    char from[FILENAME_SIZE] = ".beargit/";
    strcat(from, commit_id);
    strcat(from, "/.index");
    fs_cp(from, ".beargit/.index");

    FILE* findex1 = fopen(".beargit/.index", "r");
    char line1[FILENAME_SIZE];
    while(fgets(line1, sizeof(line1), findex1)) {
      strtok(line1, "\n");
      char temp[FILENAME_SIZE] = ".beargit/";
      strcat(temp, commit_id);
      strcat(temp, "/");
      strcat(temp, line1);
      fs_cp(temp, line1);
    }
    fclose(findex1);
  }
  else {
    fs_rm(".beargit/.index");

    FILE* findex = fopen(".beargit/.index", "w");
    fclose(findex);
  }
  write_string_to_file(".beargit/.prev", commit_id);

  return 0;
}

int is_it_a_commit_id(const char* commit_id) {
  /* COMPLETE THE REST */
  char filename[FILENAME_SIZE] = ".beargit/";
  strcat(filename, commit_id);
  strcat(filename, "/.prev");
  FILE* fout = fopen(filename, "r");
  if (fout == NULL) {
    return 0;
  } 
  fclose(fout);
  return 1;
}

int beargit_checkout(const char* arg, int new_branch) {
  // Get the current branch
  char current_branch[BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", current_branch, BRANCHNAME_SIZE);

  // If not detached, leave the current branch by storing the current HEAD into that branch's file...
  if (strlen(current_branch)) {
    char current_branch_file[BRANCHNAME_SIZE + 50];
    sprintf(current_branch_file, ".beargit/.branch_%s", current_branch);
    fs_cp(".beargit/.prev", current_branch_file);
  }

   // Check whether the argument is a commit ID. If yes, we just change to detached mode
  // without actually having to change into any other branch.
  if (is_it_a_commit_id(arg)) {
    char commit_dir[FILENAME_SIZE] = ".beargit/";
    strcat(commit_dir, arg);
    // ...and setting the current branch to none (i.e., detached).
    write_string_to_file(".beargit/.current_branch", "");

    return checkout_commit(arg);
  }

  // Read branches file (giving us the HEAD commit id for that branch).
  int branch_exists = (get_branch_number(arg) >= 0);

  // Check for errors.
  if (!(!branch_exists || !new_branch)) {
    fprintf(stderr, "ERROR:  A branch named %s already exists.\n", arg);
    return 1;
  } else if (!branch_exists && !new_branch) {
    fprintf(stderr, "ERROR:  No branch or commit %s exists.\n", arg);
    return 1;
  }

  // Just a better name, since we now know the argument is a branch name.
  char branch_name[BRANCHNAME_SIZE] = "";
  strcat(branch_name, arg);

  // File for the branch we are changing into.
  char branch_file[FILENAME_SIZE] = ".beargit/.branch_";
  strcat(branch_file, branch_name);

  // Update the branch file if new branch is created (now it can't go wrong anymore)
  if (new_branch) {
    FILE* fbranches = fopen(".beargit/.branches", "a");
    fprintf(fbranches, "%s\n", branch_name);
    fclose(fbranches);
    fs_cp(".beargit/.prev", branch_file);
  }

  write_string_to_file(".beargit/.current_branch", branch_name);

  // Read the head commit ID of this branch.
  char branch_head_commit_id[COMMIT_ID_SIZE];
  read_string_from_file(branch_file, branch_head_commit_id, COMMIT_ID_SIZE);

  // Check out the actual commit.
  return checkout_commit(branch_head_commit_id);
}

/* beargit reset
 *
 * See "Step 7" in the project spec.
 *
 */

int beargit_reset(const char* commit_id, const char* filename) {
  if (!is_it_a_commit_id(commit_id)) {
      fprintf(stderr, "ERROR:  Commit %s does not exist.\n", commit_id);
      return 1;
  }

  // Check if the file is in the commit directory
  /* COMPLETE THIS PART */
  char pathtofile[FILENAME_SIZE] = ".beargit/";
  strcat(pathtofile, commit_id);
  strcat(pathtofile, "/");
  strcat(pathtofile, filename);
  
  /////finding file in commmit folder ////
  char path[FILENAME_SIZE] = ".beargit/";
  strcat(path, commit_id);
  strcat(path, "/.index");

  FILE* findex1 = fopen(path, "r");

  int found = 0;
  char line1[FILENAME_SIZE];
  while(fgets(line1, sizeof(line1), findex1)) {
    strtok(line1, "\n");
    if (strcmp(line1, filename) == 0) {
      found = 1;
    }
  }
  fclose(findex1);
  if (found != 1){
  	fprintf(stderr, "ERROR:  %s is not in the index of commit %s.\n", filename, commit_id);
    return 1;
  }

  

  // Copy the file to the current working directory
  /* COMPLETE THIS PART */
  fs_cp(pathtofile, filename);


  // Add the file if it wasn't already there
  /* COMPLETE THIS PART */
  FILE* findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) != 0) {
	    fprintf(fnewindex, "%s\n", line);
	}
  }

  fprintf(fnewindex, "%s\n", filename);
  fclose(findex);
  fclose(fnewindex);

  fs_mv(".beargit/.newindex", ".beargit/.index");

  return 0;
}

/* beargit merge
 *
 * See "Step 8" in the project spec.
 *
 */

 int Wcontains(char filename[FILENAME_SIZE]){

  FILE* findex1 = fopen(".beargit/.index", "r");

  int found = 0;
  char line1[FILENAME_SIZE];
  while(fgets(line1, sizeof(line1), findex1)) {
    strtok(line1, "\n");
    if (strcmp(line1, filename) == 0) {
      found = 1;
    }
  }
  fclose(findex1);

  return found;
 }

int beargit_merge(const char* arg) {
  // Get the commit_id or throw an error
  char commit_id[COMMIT_ID_SIZE];
  if (!is_it_a_commit_id(arg)) {
      if (get_branch_number(arg) == -1) {
            fprintf(stderr, "ERROR:  No branch or commit %s exists.\n", arg);
            return 1;
      }
      char branch_file[FILENAME_SIZE];
      snprintf(branch_file, FILENAME_SIZE, ".beargit/.branch_%s", arg);
      read_string_from_file(branch_file, commit_id, COMMIT_ID_SIZE);
  } else {
      snprintf(commit_id, COMMIT_ID_SIZE, "%s", arg);
  }

  // Iterate through each line of the commit_id index and determine how you
  // should copy the index file over
   /* COMPLETE THE REST */

  char commitIndex[FILENAME_SIZE] = ".beargit/";
  strcat(commitIndex, commit_id);
  strcat(commitIndex, "/.index");

  FILE* findex1 = fopen(commitIndex, "r");
  char line1[FILENAME_SIZE];
  while(fgets(line1, sizeof(line1), findex1)) {
    strtok(line1, "\n");
    char WPath[FILENAME_SIZE] = "";
    strcat(WPath, line1);

    char actualPath[FILENAME_SIZE] = ".beargit/";
    strcat(actualPath, commit_id);
    strcat(actualPath, "/");
    strcat(actualPath, line1);

    if (Wcontains(line1)){
    	char tempName[FILENAME_SIZE] = "";
  
    	strcat(tempName, ".");
    	strcat(tempName, commit_id);
    	strcat(WPath, tempName); //filename.commitid

    	fs_cp(actualPath, WPath);
    	fprintf(stdout, "%s conflicted copy created\n", line1);
    } else{
    	fs_cp(actualPath, line1);
    	fprintf(stdout, "%s added\n", line1);
    	beargit_add(line1);
    }
  }
  fclose(findex1);
  return 0;
}
