
const mongoose = require('mongoose');
const axios = require("axios");
const dbs = require('./schema.js');
const utils = require('./utils.js');
const config = require('./config.js');

var db = null;
var sync_height = null;

const api_token_header = {
    headers: {'x-api-token': config.api_token}
};

var check_lock = false;

async function check()
{
    if(!sync_height) {
        return;
    }
    const height = sync_height;

    if(check_lock) {
        return;
    }
    check_lock = true;
    try {
        const list = await dbs.Payout.find({pending: true});
        for(const payout of list) {
            let failed = false;
            let expired = false;
            let confirmed = false;
            try {
                const res = await axios.get(config.node_url + '/wapi/transaction?id=' + payout.txid, api_token_header);
                const tx = res.data;
                if(tx.confirm && tx.confirm >= config.account_delay) {
                    if(tx.did_fail) {
                        failed = true;
                        console.log("Payout transaction failed with:", tx.message);
                    } else {
                        confirmed = true;
                    }
                    payout.tx_fee = tx.fee.value;
                }
            } catch(e) {
                if(height - payout.height > config.payout_tx_expire + 100) {
                    failed = true;
                    expired = true;
                    console.log("Payout transaction expired:", "height", payout.height, "txid", payout.txid);
                } else {
                    console.log("Failed to check payout transaction:", e.message,
                        "response", e.response ? e.response.data : null, "txid", payout.txid);
                }
            }
            if(!confirmed && !failed) {
                continue;
            }
            payout.valid = !failed;
            payout.pending = false;

            const conn = await db.startSession();
            try {
                await conn.startTransaction();
                const opt = {session: conn};

                if(failed) {
                    // revert balances
                    for(const entry of payout.amounts) {
                        const address = entry[0];
                        const account = await dbs.Account.findOne({address: address});
                        if(!account) {
                            throw new Error("Account not found: " + address);
                        }
                        account.balance += entry[1];
                        await account.save(opt);
                    }
                    payout.expired = expired;

                    console.log("Payout failed:", "height", payout.height, "txid", payout.txid);
                } else {
                    console.log("Payout confirmed:", "height", payout.height, "total_amount", payout.total_amount,
                        "count", payout.count, "tx_fee", payout.tx_fee, "txid", payout.txid);
                }
                await payout.save(opt);

                if(!expired) {
                    const account = await dbs.Account.findOne({address: config.fee_account});
                    if(!account) {
                        throw new Error("Fee account not found: " + config.fee_account);
                    }
                    account.balance -= payout.tx_fee;
                    await account.save(opt);
                }
                await conn.commitTransaction();
            } catch(e) {
                await conn.abortTransaction();
                throw e;
            } finally {
                conn.endSession();
            }
        }
    } catch(e) {
        console.log("check() failed:", e.message);
    } finally {
        check_lock = false;
    }
}

async function make_payout(height, amounts, opt)
{
    const options = {
        auto_send: false,
        mark_spent: true,
        expire_at: height + config.payout_tx_expire,
    };

    let tx = null;
    try {
        const res = await axios.post(config.node_url + '/wapi/wallet/send_many', {
            index: config.wallet_index,
            amounts: amounts,
            currency: "MMX",
            options: options,
        }, api_token_header);
        tx = res.data;
    } catch(e) {
        throw new Error("Failed to create payout transaction: "
            + e.message + " (" + (e.response ? e.response.data : "???") + ")");
    }

    if(tx.sender != config.pool_target) {
        throw new Error("Invalid payout transaction sender: " + tx.sender
            + " != " + config.pool_target + " (wrong config.wallet_index)");
    }

    let total_amount = 0;
    for(const entry of amounts) {
        const payout = new dbs.UserPayout({
            account: entry[0],
            height: height,
            amount: entry[1],
            txid: tx.id,
        });
        total_amount += payout.amount;
        await payout.save(opt);
    }
    const payout = new dbs.Payout({
        txid: tx.id,
        total_amount: total_amount,
        amounts: amounts,
        count: amounts.length,
        time: Date.now(),
        height: height,
        pending: true,
    });
    await payout.save(opt);

    console.log("Payout transaction created:", tx.id, "total_amount", total_amount, "count", amounts.length);
    return tx;
}

var payout_lock = false;

async function payout()
{
    if(!sync_height) {
        return;
    }
    const height = sync_height;

    if(payout_lock) {
        return;
    }
    payout_lock = true;
    try {
        const pool = await dbs.Pool.findOne({id: "this"});
        if(!pool) {
            throw new Error("Pool state not found");
        }
        if(!pool.payout_enable) {
            console.log("Payouts currently disabled");
            return;
        }
        if(pool.last_payout && height - pool.last_payout < config.payout_interval) {
            return;
        }
        const conn = await db.startSession();
        try {
            await conn.startTransaction();
            const opt = {session: conn};

            const list = dbs.Account.find({balance: {$gt: config.payout_threshold}});

            // reserve for tx fees
            const reserve = (list.length * config.tx_output_cost + 1);

            let total_count = 0;
            let total_amount = 0;
            let outputs = [];
            let tx_list = [];

            for await(const account of list)
            {
                let amount = account.balance;

                if(account.address == config.fee_account) {
                    amount -= reserve;
                    if(amount < config.payout_threshold) {
                        continue;
                    }
                }
                account.balance -= amount;
                await account.save(opt);

                console.log("Payout triggered for", account.address, "amount", amount, "MMX");

                total_count++;
                total_amount += amount;
                outputs.push([account.address, amount]);

                if(outputs.length >= config.max_payouts) {
                    tx_list.push(await make_payout(height, outputs, opt));
                    outputs = [];
                }
            }
            if(outputs.length) {
                tx_list.push(await make_payout(height, outputs, opt));
            }
            pool.last_payout = height;
            await pool.save(opt);

            await conn.commitTransaction();

            for(const tx of tx_list) {
                try {
                    await axios.post(config.node_url + '/wapi/wallet/send_off', {
                        index: config.wallet_index,
                        tx: tx,
                    }, api_token_header);
                    console.log("Payout transaction sent:", tx.id);
                } catch(e) {
                    console.log("Failed to send payout transaction:",
                        e.message, "txid", tx.id, "response", e.response ? e.response.data : "???");
                }
            }
            if(tx_list.length) {
                console.log("Payout initiated:", "height", height, "total_amount",
                    total_amount, "total_count", total_count, "tx_count", tx_list.length);
            }
        } catch(e) {
            await conn.abortTransaction();
            throw e;
        } finally {
            conn.endSession();
        }
    } catch(e) {
        console.log("payout() failed:", e.message);
    } finally {
        payout_lock = false;
    }
}

async function update()
{
    try {
        sync_height = await utils.get_synced_height();
        if(!sync_height) {
            console.log('Waiting for node sync ...');
        }
    } catch(e) {
        console.log("Failed to query sync height:", e.message);
    }
}

async function main()
{
    db = await mongoose.connect(config.mongodb_uri);

    await update();
    await payout();
    await check();

    setInterval(update, 10 * 1000);
    setInterval(payout, 15 * 60 * 1000);
    setInterval(check, 60 * 1000);
}

main();
