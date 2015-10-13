#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <CUnit/Basic.h>
#include "beargit.h"
#include "util.h"

/* printf/fprintf calls in this tester will NOT go to file. */

#undef printf
#undef fprintf

/* The suite initialization function.
 * You'll probably want to delete any leftover files in .beargit from previous
 * tests, along with the .beargit directory itself.
 *
 * You'll most likely be able to share this across suites.
 */
int init_suite(void)
{
    // preps to run tests by deleting the .beargit directory if it exists
    fs_force_rm_beargit_dir();
    unlink("TEST_STDOUT");
    unlink("TEST_STDERR");
    return 0;
}

/* You can also delete leftover files after a test suite runs, but there's
 * no need to duplicate code between this and init_suite 
 */
int clean_suite(void)
{
    return 0;
}

/* Simple test of fread().
 * Reads the data previously written by testFPRINTF()
 * and checks whether the expected characters are present.
 * Must be run after testFPRINTF().
 */
void simple_sample_test(void)
{
    // This is a very basic test. Your tests should likely do more than this.
    // We suggest checking the outputs of printfs/fprintfs to both stdout
    // and stderr. To make this convenient for you, the tester replaces
    // printf and fprintf with copies that write data to a file for you
    // to access. To access all output written to stdout, you can read 
    // from the "TEST_STDOUT" file. To access all output written to stderr,
    // you can read from the "TEST_STDERR" file.
    int retval;
    retval = beargit_init();
    CU_ASSERT(0==retval);
    retval = beargit_add("asdf.txt");
    CU_ASSERT(0==retval);
}

struct commit {
  char msg[MSG_SIZE];
  struct commit* next;
};


void free_commit_list(struct commit** commit_list) {
  if (*commit_list) {
    free_commit_list(&((*commit_list)->next));
    free(*commit_list);
  }

  *commit_list = NULL;
}

void run_commit(struct commit** commit_list, const char* msg) {
    int retval = beargit_commit(msg);
    CU_ASSERT(0==retval);

    struct commit* new_commit = (struct commit*)malloc(sizeof(struct commit));
    new_commit->next = *commit_list;
    strcpy(new_commit->msg, msg);
    *commit_list = new_commit;
}

void simple_log_test(void)
{
    struct commit* commit_list = NULL;
    int retval;
    retval = beargit_init();
    CU_ASSERT(0==retval);
    FILE* asdf = fopen("asdf.txt", "w");
    fclose(asdf);
    retval = beargit_add("asdf.txt");
    CU_ASSERT(0==retval);
    run_commit(&commit_list, "THIS IS BEAR TERRITORY!1");
    run_commit(&commit_list, "THIS IS BEAR TERRITORY!2");
    run_commit(&commit_list, "THIS IS BEAR TERRITORY!3");

    retval = beargit_log(10);
    CU_ASSERT(0==retval);

    struct commit* cur_commit = commit_list;

    const int LINE_SIZE = 512;
    char line[LINE_SIZE];

    FILE* fstdout = fopen("TEST_STDOUT", "r");
    CU_ASSERT_PTR_NOT_NULL(fstdout);

    while (cur_commit != NULL) {
      char refline[LINE_SIZE];

      // First line is commit -- don't check the ID.
      CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
      CU_ASSERT(!strncmp(line,"commit", strlen("commit")));

      // Second line is msg
      sprintf(refline, "   %s\n", cur_commit->msg);
      CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
      CU_ASSERT_STRING_EQUAL(line, refline);

      // Third line is empty
      CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
      CU_ASSERT(!strcmp(line,"\n"));

      cur_commit = cur_commit->next;
    }

    CU_ASSERT_PTR_NULL(fgets(line, LINE_SIZE, fstdout));

    // It's the end of output
    CU_ASSERT(feof(fstdout));
    fclose(fstdout);

    free_commit_list(&commit_list);
}

//this test tests reset. It checks for the two erroring cases
//It checks if reset replaces the version of the file in the 
//working directory with the specified file for reset.
//it also tests if index is update and file is sucessfully
//reset if the file is not in the working directory.
void reset_test(void){
	struct commit* commit_list = NULL;
    int retval;
    retval = beargit_init();
    CU_ASSERT(0==retval);

    write_string_to_file("wug.txt", "Dis a wug");
    write_string_to_file("wug2.txt", "Der two wugz");
    write_string_to_file("wug3.txt", "Dis not a wug");

    retval = beargit_add("wug.txt");
    CU_ASSERT(0==retval);
    run_commit(&commit_list, "THIS IS BEAR TERRITORY!1");

    retval = beargit_add("wug2.txt");
    CU_ASSERT(0==retval);
    run_commit(&commit_list, "THIS IS BEAR TERRITORY!2");

    retval = beargit_add("wug3.txt");
    CU_ASSERT(0==retval);
    run_commit(&commit_list, "THIS IS BEAR TERRITORY!3");

    char commit_id[512] = "";
    read_string_from_file(".beargit/.prev", commit_id, 512);

    //Check for error if commit doens't exit
    retval = beargit_reset("alskdfjlaskfj", "wug.txt");
    CU_ASSERT(1==retval); //first line of stderr: "ERROR:  Commit alskdfjlaskfj does not exist.\n"

    //Check for error if file not in commit
    retval = beargit_reset(commit_id, "wug6.txt");
    CU_ASSERT(1==retval); //second line of stderr: "ERROR:  wug6.txt is not in the index of commit [insert commit_id].\n"

    //modifies wug.txt in working directory
    write_string_to_file("wug.txt", "JK dis not a wug");

    //Resets wug.txt from commit 3
    retval = beargit_reset(commit_id, "wug.txt");
    CU_ASSERT(0==retval);
    char stuffinwug[512] = "";
    read_string_from_file("wug.txt", stuffinwug, 512);
    CU_ASSERT(strcmp(stuffinwug, "Dis a wug") == 0);

    //remove wug3.txt
    fs_rm("wug3.txt");
    retval = beargit_rm("wug3.txt");
    CU_ASSERT(retval == 0);
	FILE* fstdout_wug = fopen("wug3.txt", "r");
    CU_ASSERT_PTR_NULL(fstdout_wug);


    //try to reset wug3.txt
    retval = beargit_reset(commit_id, "wug3.txt");
    FILE* check_wug_exist = fopen("wug3.txt", "r");
    CU_ASSERT_PTR_NOT_NULL(check_wug_exist);
    char check_index_4wug[512] = "";
    read_string_from_file(".beargit/.index", check_index_4wug, 512);
    char PostToBe[512] = "wug2.txt\nwug.txt\nwug3.txt\n";
    CU_ASSERT(strcmp(check_index_4wug, PostToBe) == 0);
  	fclose(check_wug_exist);

	//Read stderr file  

    char PostToBe4Err[512] = "";
    strcat(PostToBe4Err, "ERROR:  Commit alskdfjlaskfj does not exist.\nERROR:  wug6.txt is not in the index of commit ");
    strcat(PostToBe4Err, commit_id);
    strcat(PostToBe4Err, ".\n");

    char ActualErr[512] ="";
    read_string_from_file("TEST_STDERR", ActualErr, 512);
    CU_ASSERT(strcmp(PostToBe4Err, ActualErr) == 0);

    free_commit_list(&commit_list);
}


//This suite tests merging multiple files. It first tests for error cases, then 
//it tests for non-conflicting files then test for conflicted files.
void merge_test(void){

	struct commit* commit_list = NULL;
    int retval;
    retval = beargit_init();
    CU_ASSERT(0==retval);

    write_string_to_file("wug.txt", "Dis a wug");
    write_string_to_file("wug1.txt", "Dis a wug 1.5");
    write_string_to_file("wug2.txt", "Der two wugz");
    write_string_to_file("wug3.txt", "Dis not a wug");

    retval = beargit_add("wug.txt");
    CU_ASSERT(0==retval);
    retval = beargit_add("wug1.txt");
    CU_ASSERT(0==retval);
    run_commit(&commit_list, "THIS IS BEAR TERRITORY!1");

    //save commit1's ID
    char commit1_id[512] = "";
    read_string_from_file(".beargit/.prev", commit1_id, 512);

    retval = beargit_add("wug2.txt");
    CU_ASSERT(0==retval);
    run_commit(&commit_list, "THIS IS BEAR TERRITORY!2");

    //remove wug, wug1 and wug2
    fs_rm("wug.txt");
    retval = beargit_rm("wug.txt");
    CU_ASSERT(0==retval);    

    fs_rm("wug1.txt");
    retval = beargit_rm("wug1.txt");
    CU_ASSERT(0==retval);    

    fs_rm("wug2.txt");
    retval = beargit_rm("wug2.txt");
    CU_ASSERT(0==retval);

    //add wug3 & commit
    retval = beargit_add("wug3.txt");
    CU_ASSERT(0==retval);
    run_commit(&commit_list, "THIS IS BEAR TERRITORY!3");

    //Check for err msg when merging non existent commit ID / brench
    retval = beargit_merge("alskdfjlaskfj");
    CU_ASSERT(1==retval); //first line of stderr: "ERROR:  No branch or commit alskdfjlaskfj exists."

    //test merging non-conflicted files
    retval = beargit_merge(commit1_id);
    CU_ASSERT(0==retval); //first line of stdout: "wug.txt added\nwug1.txt added\n"
    //check files actually moved
    FILE* fstdout_wug = fopen("wug.txt", "r");
    CU_ASSERT_PTR_NOT_NULL(fstdout_wug);
    FILE* fstdout_wug1 = fopen("wug1.txt", "r");
    CU_ASSERT_PTR_NOT_NULL(fstdout_wug1);
    fclose(fstdout_wug);
    fclose(fstdout_wug1);


    //test merging conflicted files
    retval = beargit_merge(commit1_id);
    CU_ASSERT(0==retval); //first line of stdout: "wug.txt copy created \nwug1.txt copy created\n"
    //check conflicted files created
    char wugc[512] = "wug.txt.";
    strcat(wugc, commit1_id);
    char wugc1[512] = "wug1.txt.";
    strcat(wugc1, commit1_id);

    FILE* fstdout_wugc = fopen(wugc, "r");
    CU_ASSERT_PTR_NOT_NULL(fstdout_wugc);
    FILE* fstdout_wugc1 = fopen(wugc1, "r");
    CU_ASSERT_PTR_NOT_NULL(fstdout_wugc1);
    fclose(fstdout_wugc);
    fclose(fstdout_wugc1);

	//Read stderr file  
    char PostToBe4Err[512] = "ERROR:  No branch or commit alskdfjlaskfj exists.\n";

    char ActualErr[512] ="";
    read_string_from_file("TEST_STDERR", ActualErr, 512);
    CU_ASSERT(strcmp(PostToBe4Err, ActualErr) == 0);

    // read stdout
    char PostToBe[512] = "wug.txt added\nwug1.txt added\nwug.txt copy created\nwug1.txt copy created\n";

    char Actual[512] ="";
    read_string_from_file("TEST_STDOUT", Actual, 512);
    CU_ASSERT(strcmp(PostToBe, Actual) == 0);


	free_commit_list(&commit_list);
}


/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int cunittester()
{
   CU_pSuite pSuite = NULL;
   CU_pSuite pSuite2 = NULL;
   CU_pSuite pSuite3 = NULL;
   CU_pSuite pSuite4 = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("Suite_1", init_suite, clean_suite);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Add tests to the Suite #1 */
   if (NULL == CU_add_test(pSuite, "Simple Test #1", simple_sample_test))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   pSuite2 = CU_add_suite("Suite_2", init_suite, clean_suite);
   if (NULL == pSuite2) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Add tests to the Suite #2 */
   if (NULL == CU_add_test(pSuite2, "Log output test", simple_log_test))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   pSuite3 = CU_add_suite("Suite_3", init_suite, clean_suite);
   if (NULL == pSuite3) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Add tests to the Suite #3 */
   if (NULL == CU_add_test(pSuite3, "Reset test", reset_test))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   pSuite4 = CU_add_suite("Suite_4", init_suite, clean_suite);
   if (NULL == pSuite4) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Add tests to the Suite #4 */
   if (NULL == CU_add_test(pSuite4, "Merge test", merge_test))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}

