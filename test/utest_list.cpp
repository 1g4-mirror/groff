// -*- C++ -*-
/* Copyright (C) 2016  Free Software Foundation, Inc.
     Written by Bertrand Garrigues (bertrand.garrigues@laposte.net)

This file is part of groff.

groff is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or
(at your option) any later version.

groff is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>. */

#include "config.h"
#include "list.h"
#include <iostream>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

/**
 * A class to test the struct list_head implementation of double-linked list.
 */
class dummy {
public:
  int data;
  struct list_head lh;
  dummy (int x)
  {
    data = x;
    INIT_LIST_HEAD(&lh, this);
  }

  int get_data ()
  {
    return data;
  }
};

class ListTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ListTest);
  CPPUNIT_TEST(testAdd);
  CPPUNIT_TEST(testAddTail);
  CPPUNIT_TEST(testForEachEntry);
  CPPUNIT_TEST(testForEachEntry2);
  CPPUNIT_TEST(testForEachEntrySafe);
  CPPUNIT_TEST(testForEachEntrySafe2);
  CPPUNIT_TEST_SUITE_END();

private:
  // Head of the list: we will add class dummy object to this list.
  struct list_head head;
  // dummy objects to be added to the list.
  dummy *a, *b, *c;
public:
  // Fixture
  void setUp()
  {
    INIT_LIST_HEAD(&head, NULL);
  }

  // Teardown: we rather make individual teardown at the end of each test.
  void tearDown()
  {
  }

  // list_add: this should add a new dummy object after head, like in a stack.
  void testAdd()
  {
    a = new dummy(10);
    b = new dummy(20);
    c = new dummy(30);

    list_add(&a->lh, &head);
    CPPUNIT_ASSERT(head.next == &a->lh);
    CPPUNIT_ASSERT(head.prev == &a->lh);
    list_add(&b->lh, &head);
    CPPUNIT_ASSERT(head.next == &b->lh);
    CPPUNIT_ASSERT(head.prev == &a->lh);
    list_add(&c->lh, &head);
    CPPUNIT_ASSERT(head.next == &c->lh);
    CPPUNIT_ASSERT(head.prev == &a->lh);

    list_del_init(&a->lh);
    list_del_init(&b->lh);
    list_del_init(&c->lh);
    CPPUNIT_ASSERT(list_empty(&head));

    // Local teardown
    INIT_LIST_HEAD(&head, NULL);
    delete a;
    delete b;
    delete c;
  }

  // list_add: this should add a new dummy object before head, like in a queue.
  void testAddTail()
  {
    a = new dummy(10);
    b = new dummy(20);
    c = new dummy(30);

    list_add_tail(&a->lh, &head);
    CPPUNIT_ASSERT(head.next == &a->lh);
    CPPUNIT_ASSERT(head.prev == &a->lh);
    list_add_tail(&b->lh, &head);
    CPPUNIT_ASSERT(head.next == &a->lh);
    CPPUNIT_ASSERT(head.prev == &b->lh);
    list_add_tail(&c->lh, &head);
    CPPUNIT_ASSERT(head.next == &a->lh);
    CPPUNIT_ASSERT(head.prev == &c->lh);

    list_del_init(&a->lh);
    list_del_init(&b->lh);
    list_del_init(&c->lh);
    CPPUNIT_ASSERT(list_empty(&head));

    // Local teardown
    INIT_LIST_HEAD(&head, NULL);
    delete a;
    delete b;
    delete c;
  }

  // Walk into the list and display the data of each dummy object
  void testForEachEntry()
  {
    dummy *pos;
    int k = 10;

    a = new dummy(10);
    b = new dummy(20);
    c = new dummy(30);

    list_add_tail(&a->lh, &head);
    list_add_tail(&b->lh, &head);
    list_add_tail(&c->lh, &head);
    list_for_each_entry(pos, &head, lh, dummy) {
      CPPUNIT_ASSERT(pos->data == k);
      k+=10;
    }

    CPPUNIT_ASSERT(k == 40);

    // Local teardown
    INIT_LIST_HEAD(&head, NULL);
    delete a;
    delete b;
    delete c;
  }

  // Special case of a list of size 1 or an empty list
  void testForEachEntry2()
  {
    dummy *pos;
    a = new dummy(10);

    list_add_tail(&a->lh, &head);
    list_for_each_entry(pos, &head, lh, dummy) {
      CPPUNIT_ASSERT(pos->data == 10);
    }
    list_del_init(&a->lh);

    list_for_each_entry(pos, &head, lh, dummy) {
      // We should not enter this loop
      CPPUNIT_ASSERT(false);
    }
    CPPUNIT_ASSERT(true);

    // Local teardown
    INIT_LIST_HEAD(&head, NULL);
    delete a;
  }

  // Walk into the list, but this time we delete the dummy objects one by one,
  // so we use list_for_each_entry_safe rather than list_for_each_entry
  void testForEachEntrySafe()
  {
    dummy *pos, *n;
    int k = 10;

    a = new dummy(10);
    b = new dummy(20);
    c = new dummy(30);

    list_add_tail(&a->lh, &head);
    list_add_tail(&b->lh, &head);
    list_add_tail(&c->lh, &head);
    list_for_each_entry_safe(pos, n, &head, lh, dummy) {
      list_del_init(&pos->lh);
      CPPUNIT_ASSERT(pos->data == k);
      k+=10;
      delete pos;
    }

    CPPUNIT_ASSERT(true);
    // Local teardown
    INIT_LIST_HEAD(&head, NULL);
  }

  // Just test that we don't crash when walking thru a list with 1 item or an
  // empty list.
  void testForEachEntrySafe2()
  {
    dummy *pos, *n;
    a = new dummy(10);

    list_add_tail(&a->lh, &head);
    list_for_each_entry_safe(pos, n, &head, lh, dummy) {
      list_del_init(&pos->lh);
      CPPUNIT_ASSERT(pos->data == 10);
      delete pos;
    }

    list_for_each_entry_safe(pos, n, &head, lh, dummy) {
      // we should not enter this loop
      CPPUNIT_ASSERT(false);
    }

    CPPUNIT_ASSERT(true);
    // Local teardown
    INIT_LIST_HEAD(&head, NULL);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ListTest);

int main(int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  bool wasSuccessful;
  CppUnit::TestFactoryRegistry &registry =
    CppUnit::TestFactoryRegistry::getRegistry();

  runner.addTest(registry.makeTest());
  wasSuccessful = runner.run("", false);

  return !wasSuccessful;
}
