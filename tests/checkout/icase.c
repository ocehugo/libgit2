#include "clar_libgit2.h"

#include "git2/checkout.h"
#include "path.h"

static git_repository *repo;
static git_object *obj;
static git_checkout_options checkout_opts;

void test_checkout_icase__initialize(void)
{
	git_oid id;

	repo = cl_git_sandbox_init("testrepo");

	cl_git_pass(git_reference_name_to_id(&id, repo, "refs/heads/dir"));
	cl_git_pass(git_object_lookup(&obj, repo, &id, GIT_OBJ_ANY));

	git_checkout_init_options(&checkout_opts, GIT_CHECKOUT_OPTIONS_VERSION);
	checkout_opts.checkout_strategy = GIT_CHECKOUT_FORCE;
}

void test_checkout_icase__cleanup(void)
{
	git_object_free(obj);
	cl_git_sandbox_cleanup();
}

static void assert_name_is(const char *expected)
{
	char *actual;
	size_t actual_len, expected_len, start;

	cl_assert(actual = realpath(expected, NULL));

	expected_len = strlen(expected);
	actual_len = strlen(actual);
	cl_assert(actual_len >= expected_len);

	start = actual_len - expected_len;
	cl_assert_equal_s(expected, actual + start);

	if (start)
		cl_assert_equal_strn("/", actual + (start - 1), 1);

	free(actual);
}

void test_checkout_icase__overwrites_files_for_files(void)
{
	cl_git_write2file("testrepo/NEW.txt", "neue file\n", 10, \
		O_WRONLY | O_CREAT | O_TRUNC, 0644);

	cl_git_pass(git_checkout_tree(repo, obj, &checkout_opts));
	assert_name_is("testrepo/new.txt");
}

void test_checkout_icase__overwrites_links_for_files(void)
{
	cl_must_pass(p_symlink("../tmp", "testrepo/NEW.txt"));

	cl_git_pass(git_checkout_tree(repo, obj, &checkout_opts));

	cl_assert(!git_path_exists("tmp"));
	assert_name_is("testrepo/new.txt");
}

void test_checkout_icase__overwites_folders_for_files(void)
{
	cl_must_pass(p_mkdir("testrepo/NEW.txt", 0777));

	cl_git_pass(git_checkout_tree(repo, obj, &checkout_opts));

	assert_name_is("testrepo/new.txt");
	cl_assert(!git_path_isdir("testrepo/new.txt"));
}

void test_checkout_icase__overwrites_files_for_folders(void)
{
	cl_git_write2file("testrepo/A", "neue file\n", 10, \
		O_WRONLY | O_CREAT | O_TRUNC, 0644);

	cl_git_pass(git_checkout_tree(repo, obj, &checkout_opts));
	assert_name_is("testrepo/a");
	cl_assert(git_path_isdir("testrepo/a"));
}

void test_checkout_icase__overwrites_links_for_folders(void)
{
	cl_must_pass(p_symlink("..", "testrepo/A"));

	cl_git_pass(git_checkout_tree(repo, obj, &checkout_opts));

	cl_assert(!git_path_exists("b.txt"));
	assert_name_is("testrepo/a");
}

